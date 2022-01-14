#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ctrace.h>


#define TRC_PORT 2013

#define SYSFILE(a) ((a == _STDOUT) || (a == _STDERR))  

tenable_t _trc = TRC_DISABLED;		/* trace enabled ? */
static ton_t _on = TRC_OFF;		/* trace on */	
static tlevel_t _tlevel = TRC_NONE;	/* trace levels enabled */
static FILE *_fp = NULL;		/* trc msgs output file stream */
static tserver_t  _server = 0;		/* run trace library as separate thread */
static int _msgs = 0;			/* msgs still being processed by server */
static pthread_t _sid;			/* server thread id */
static int _serv_sockfd;		/* server socket */
static sem_t _startclient;		/* dont start client until server inited */
static int _initialised = 0;		/* static trace library initialised */


/* trace field sizes */ 

#define T1	30		/* file name 	*/
#define T2	4		/* line number  */
#define T3	10		/* thread name  */
#define T4	30		/* keyword max 	*/
#define T5	1024		/* info max 	*/
#define TRACELEN (T1 + T2 + T3 + T4 + T5) 

/* indented trace fields start at WSPACEMAX + 1 */
#define WSPACEMAX (T1 + T2 + T3)	
#define WSPACELEN(a) WSPACEMAX - a


/* dont create thread data buffers on the fly */
/* instead keep them permanently in memory as */
/* part of the tthread_t struct */

/* trace thread type */
typedef struct tthread_t{
	tid_t id;
	char name[T3];	
	int on;
	tlevel_t level;
	char wspace[WSPACEMAX];	
	char fmt[T5];
	char trace[TRACELEN];
	struct tthread_t *next;	
} tthread_t;

#define THRDMAX	307	/* keep this prime - used in hashing fn */ 
#define HASH(a) a % THRDMAX 

static tthread_t *_thread[THRDMAX] = {0}; 	/* trace thread hash lookup table */
static uint_t _numthreads = 0;	
static pthread_mutex_t _hashmutex;	/* for _thread hash table integrity */
static int _hashreads = 0;
static sem_t _hashsem;

/* trace unit type */
typedef struct tunit_t{
	int on;
	tlevel_t level;
}tunit_t;

#define UNITMAX 1000
static tunit_t *_unit = NULL;		/* trace unit array */
static uint_t _unitmax = 0;		/* size of trace unit array */

/* write has exclusive use of hash table */
/* muliple reads can occur concurrently */
#define HASH_READ_ENTER() \
	pthread_mutex_lock(&_hashmutex); \
	if(!_hashreads) \
		sem_wait(&_hashsem); \
	_hashreads++; \
	pthread_mutex_unlock(&_hashmutex);

#define HASH_READ_EXIT() \
	pthread_mutex_lock(&_hashmutex); \
	--_hashreads; \
	if(!_hashreads) \
		sem_post(&_hashsem); \
	pthread_mutex_unlock(&_hashmutex);

#define HASH_WRITE_ENTER() sem_wait(&_hashsem)
#define HASH_WRITE_EXIT() sem_post(&_hashsem)


/* functions requiring forward delcarations */
int trc_file(cchar_t *file);	
void *trc_start_server(void);
int trc_stop_server();

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_init(		// function name
   cchar_t *file,	// output stream(file, stdout, stderr), NULL=stdout  
   tenable_t trc,	// trace enabled flag
   ton_t on,		// trace on flag
   tlevel_t level,	// trace levels to default to
   tserver_t server)	// if server=1 run trace as separate server
   --------------------------------------------------------------------*/
int trc_init(cchar_t *file, tenable_t trc, ton_t on, tlevel_t level, uint_t umax, tserver_t server)
{
	int i, ret = 0;

	if(_initialised) return 1;

	_trc = trc;
	_on = on;
	_tlevel = level;
	_server = server;

	if(trc_file(file))
		return 1;
	
	for(i=0; i<THRDMAX; i++) {
		_thread[i] = NULL;
	}
	/* initialise logical software unit array */
	if(umax > 0){
		_unitmax = umax;
		_unit = (tunit_t *)malloc(sizeof(tunit_t)*_unitmax);
		for(i=0; i<_unitmax; i++){
			_unit[i].on = TRC_OFF;
			_unit[i].level = TRC_ALL;
		}
	}

	if(pthread_mutex_init(&_hashmutex, NULL))
		return 1;

	sem_init(&_hashsem, 0, 1);
	sem_init(&_startclient, 0, 1);

	/* run trace as separate thread */
	if(_server == TRC_SERV_ON){
		sem_wait(&_startclient);
		if(pthread_create(&_sid, NULL, trc_start_server, NULL)){
			trc_end();
			return 1;	
		}
		trc_start_client();
	}

	_initialised = 1;

	return 0;
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_end()		// procedure name
   --------------------------------------------------------------------*/
void trc_end()
{
	int i;
	tthread_t *cur, *next;

	_trc = TRC_DISABLED;

	if(_server){
		trc_stop_server();
		pthread_join(_sid, NULL);
		trc_stop_client();
	}

	for(i=0; i<THRDMAX; i++){
		for(cur=_thread[i]; cur !=NULL; cur = next){
			next = cur->next;
			free(cur);
		}
	}

	pthread_mutex_destroy(&_hashmutex);
	sem_destroy(&_hashsem);
	sem_destroy(&_startclient);

	if(!SYSFILE(_fp))
		fclose(_fp);

	_initialised = 0;
}


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_file(		// function name
   cchar_t *file)	// output stream(file, stdout, stderr), NULL=stdout  
   --------------------------------------------------------------------*/
int trc_file(cchar_t *file)
{
	if(_fp && !SYSFILE(_fp))
		fclose(_fp);
	
	if(!file)
		_fp = _STDOUT;
	else if(!(_fp = fopen(file, "w")))
		return 1; 

	return 0;
}

/* ----------------------------------------------------------------------
   tthread_t *		// thread struct of global data   
   trc_thread(		// function name
   tid_t id)		// thread id for index into _threads hash table 
   --------------------------------------------------------------------*/
tthread_t *trc_thread(tid_t id)
{
	tthread_t *t = NULL;
	
	/* if thread already added return thread */
	for(t=_thread[HASH((int)id)]; t!=NULL; t=t->next)
		if(t->id == id)
			break;
	return t; 
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_print()		// procedure name
   desc: Print all current added threads, for debugging purposes.
   --------------------------------------------------------------------*/
void trc_print_threads()
{
	int i;
	tthread_t *t = NULL;

	HASH_WRITE_ENTER();
	for(i=0; i<THRDMAX; i++)
		for(t=_thread[i]; _thread[i] && t!=NULL; t=t->next)
			printf("t->id: %d\t t->name: %s\t t->level: %d\tt->on: %d\n", 
				t->id, t->name, t->level, t->on);
	HASH_WRITE_EXIT();
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_add_thread(	// function name
   cchar_t *tname,	// trace output thread name 
   tid_t id)		// thread id
   desc: add thread struct to _thread hash table for thread with <id>
   --------------------------------------------------------------------*/
int trc_add_thread(cchar_t *tname, tid_t id)
{
	uint_t i;
	tthread_t *t = NULL;

	if(id == 0)
		id = pthread_self();

	HASH_WRITE_ENTER();
	if(t = trc_thread(id)){
		//strncpy(t->name, tname, T3);
		if(tname)
			sprintf(t->name, "%s", tname);
		else
			sprintf(t->name, "%d", id);
	}
	else{

		t = (tthread_t *)malloc(sizeof(tthread_t));
		if(!t){
			HASH_WRITE_EXIT();
			return 1;
		}

		i = HASH((unsigned int)id);
		t->id = id;
		//strncpy(t->name, tname, T3);
		if(tname)
			sprintf(t->name, "%s", tname);
		else
			sprintf(t->name, "%d", id);
		t->on = _on; 
		t->level = _tlevel;
		memset(t->wspace, ' ', WSPACEMAX);
		
		t->next = _thread[i];
		_thread[i] = t;
		_numthreads++;
	}
	HASH_WRITE_EXIT();

	return 0;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_remove_thread(	// function name
   tid_t id)		// thread id
   desc: remove thread struct from _thread hash table for thread with <id>
   --------------------------------------------------------------------*/
int trc_remove_thread(tid_t id)
{
	int i, ret = 1;
	tthread_t *cur, *next;

	if(id == 0)
		id = pthread_self();
	
	HASH_WRITE_ENTER();
	i = HASH((int)id);

	// if head of list is target, remove head
	if(_thread[i] && _thread[i]->id == id){
		cur = _thread[i];
		_thread[i] = cur->next;
		free(cur);
		_numthreads--;
		ret = 0;
	}
	// else we need to find and remove target
	else{
		for(cur=_thread[i]; cur!=NULL; cur=cur->next){
			next = cur->next;
			if(next && (next->id == id)){
				cur->next = next->next;
				free(next);
				_numthreads--;
				ret = 0;
			}
		}
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_turn_on()	// procedure name
   desc: turn tracing on for all added threads 
   --------------------------------------------------------------------*/
void trc_turn_on()
{
	uint_t i;
	tthread_t *t = NULL;

	HASH_WRITE_ENTER();
	_on = TRC_ON;
	for(i=0; i<THRDMAX; i++)
		for(t=_thread[i]; _thread[i] && t!=NULL; t=t->next)
			t->on = 1;
	HASH_WRITE_EXIT();
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_turn_off()	// procedure name
   desc: turn tracing on for all added threads 
   --------------------------------------------------------------------*/
void trc_turn_off()
{
	uint_t i;
	tthread_t *t = NULL;

	HASH_WRITE_ENTER();
	_on = TRC_OFF;
	for(i=0; i<THRDMAX; i++)
		for(t=_thread[i]; _thread[i] && t!=NULL; t=t->next)
			t->on = 0;
	HASH_WRITE_EXIT();
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_turn_thread_on(	// function name
   tid_t id)		// thread id
   desc: turn tracing on for thread <id> 
   --------------------------------------------------------------------*/
int trc_turn_thread_on(tid_t id)
{
	int ret = 1;
	tthread_t *t = NULL;

	if(id == 0)
		id = pthread_self();

	HASH_READ_ENTER();
	if(t = trc_thread(id)){
		t->on = TRC_ON;
		ret = 0;
	}
	HASH_READ_EXIT();
	
	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_turn_thread_off(	// function name
   tid_t id)		// thread id
   desc: turn tracing off for thread <id> 
   --------------------------------------------------------------------*/
int trc_turn_thread_off(tid_t id)
{
	int ret = 1;
	tthread_t *t = NULL;

	if(id == 0)
		id = pthread_self();

	HASH_READ_ENTER();
	if(t = trc_thread(id)){
		t->on = TRC_OFF;
		ret = 0;
	}
	HASH_READ_EXIT();
	
	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure 
   trc_turn_unit_on(	// procedure name
   tunit_t *tl)	// unit software section variable
   desc: turn tracing on for unit section of software 
   --------------------------------------------------------------------*/
int trc_turn_unit_on(uint_t i)
{
	int ret = 1;

	if(_unitmax == 0 || i<0 || i>=_unitmax)
		return ret;

	/* this wont happen often so just reuse hash table locks */
	HASH_WRITE_ENTER();
	if(i < UNITMAX){
		_unit[i].on = TRC_ON;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure 
   trc_turn_unit_off(	// procedure name
   tunit_t *tl)	// unit software section variable
   desc: turn tracing off for unit section of software 
   --------------------------------------------------------------------*/
int trc_turn_unit_off(uint_t i)
{
	int ret = 1;

	if(_unitmax == 0 || i<0 || i>=_unitmax)
		return ret;

	/* this wont happen often so just reuse hash table locks */
	HASH_WRITE_ENTER();
	if(i < UNITMAX){
		_unit[i].on = TRC_OFF;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=false, 1=true
   trc_valid_level(	// function name
   tlevel_t tlevel)	// trace level(s) 
   desc: validate tlevel 
   --------------------------------------------------------------------*/
int trc_valid_level(tlevel_t tlevel)
{
	return ((tlevel & ~(TRC_ALL)) == 0) ;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_set_level(	// function name
   tlevel_t tlevel)	// trace level(s)
   desc: set level=<level> in all added threads 
   --------------------------------------------------------------------*/
int trc_set_level(tlevel_t level)
{
	uint_t i;
	tthread_t *t = NULL;

	if(!trc_valid_level(level))
		return 1;

	HASH_WRITE_ENTER();
	_tlevel = level;
	for(i=0; i<THRDMAX; i++)
		for(t=_thread[i]; _thread[i] && t!=NULL; t=t->next)
			t->level = level;
	HASH_WRITE_EXIT();

	return 0;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_add_level(	// function name
   tlevel_t tlevel)	// trace level(s)
   desc: add <level> to all added threads 
   --------------------------------------------------------------------*/
int trc_add_level(tlevel_t level)
{
	uint_t i;
	tthread_t *t = NULL;

	if(!trc_valid_level(level)) return 1;

	HASH_WRITE_ENTER();
	_tlevel |= level;
	for(i=0; i<THRDMAX; i++)
		for(t=_thread[i]; _thread[i] && t!=NULL; t=t->next)
			t->level |= level;
	HASH_WRITE_EXIT();

	return 0;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_remove_level(	// function name
   tlevel_t tlevel)	// trace level(s)
   desc: remove <level> in all added threads 
   --------------------------------------------------------------------*/
int trc_remove_level(tlevel_t level)
{
	uint_t i;
	tthread_t *t = NULL;

	if(!trc_valid_level(level)) return 1;

	HASH_WRITE_ENTER();
	_tlevel &= ~level;
	for(i=0; i<THRDMAX; i++)
		for(t=_thread[i]; _thread[i] && t!=NULL; t=t->next)
			t->level &= ~level;
	HASH_WRITE_EXIT();

	return 0;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_set_thread_level(	// function name
   tlevel_t tlevel	// trace level(s)
   tid_t id)		// thread id
   desc: set level=<level> in thread <id>
   --------------------------------------------------------------------*/
int trc_set_thread_level(tlevel_t level, tid_t id)
{
	int ret = 1;
	tthread_t *t = NULL;

	if(id == 0)
		id = pthread_self();

	HASH_WRITE_ENTER();
	if((t = trc_thread(id)) && trc_valid_level(level)){
		t->level = level;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_add_thread_level(	// function name
   tlevel_t tlevel	// trace level(s)
   tid_t id)		// thread id
   desc: add <level> to thread <id>
   --------------------------------------------------------------------*/
int trc_add_thread_level(tlevel_t level, tid_t id)
{
	int ret = 1;
	tthread_t *t = NULL;

	if(id == 0)
		id = pthread_self();

	HASH_WRITE_ENTER();
	if((t = trc_thread(id)) && trc_valid_level(level)){
		t->level |= level;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_remove_thread_level(	// function name
   tlevel_t tlevel	// trace level(s)
   tid_t id)		// thread id
   desc: remove <level> from thread <id>
   --------------------------------------------------------------------*/
int trc_remove_thread_level(tlevel_t level, tid_t id)
{
	int ret = 1;
	tthread_t *t = NULL;

	if(id == 0)
		id = pthread_self();

	HASH_WRITE_ENTER();
	if((t = trc_thread(id)) && trc_valid_level(level)){
		t->level &= ~level;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_set_unit_level(	// function name
   tunit_t *tl		// trace level(s)
   tlevel_t level)	// tlevel_t 
   desc: set level=<level> in tunit_t <tl>
   --------------------------------------------------------------------*/
int trc_set_unit_level(uint_t i, tlevel_t level)
{
	int ret = 1;

	if(_unitmax == 0 || i<0 || i>=_unitmax)
		return ret;

	/* this wont happen often so just reuse hash table locks */
	HASH_WRITE_ENTER();
	if(i < UNITMAX && trc_valid_level(level)){
		_unit[i].level = level;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_add_unit_level(	// function name
   tunit_t *tl		// trace level(s)
   tlevel_t level)	// tlevel_t 
   desc: add level=<level> in tunit_t <tl>
   --------------------------------------------------------------------*/
int trc_add_unit_level(uint_t i, tlevel_t level)
{
	int ret = 1;

	if(_unitmax == 0 || i<0 || i>=_unitmax)
		return ret;

	/* this wont happen often so just reuse hash table locks */
	HASH_WRITE_ENTER();
	if(i < UNITMAX  && trc_valid_level(level)){
		_unit[i].level |= level;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   trc_remove_unit_level(// function name
   tunit_t *tl		// trace level(s)
   tlevel_t level)	// tlevel_t 
   desc: remove level=<level> from tunit_t <tl>
   --------------------------------------------------------------------*/
int trc_remove_unit_level(uint_t i, tlevel_t level)
{
	int ret = 1;

	if(_unitmax == 0 || i<0 || i>=_unitmax)
		return ret;

	/* this wont happen often so just reuse hash table locks */
	HASH_WRITE_ENTER();
	if(i < UNITMAX && trc_valid_level(level)){
		_unit[i].level &= ~level;
		ret = 0;
	}
	HASH_WRITE_EXIT();

	return ret;
}


/* ----------------------------------------------------------------------
   void			// procedure 
   trc_state()		// procedure name
   desc: print trace state info for debugging purposes.
   --------------------------------------------------------------------*/
void trc_state()
{
	printf("_trc: %d\t_level: %d\t_t: %d\t", _trc, _tlevel, _server);
	if(_fp == _STDOUT)
		printf("_fp: _STDOUT");
	else if(_fp)
		printf("_fp: user file");
	else
		printf("_fp: NULL");
	printf("\n");
}

/* ----------------------------------------------------------------------
   char *		// string output of fmt/argslist combination 
   trc_varargs(		// function name
   const char *fmt,	// format
   ...)			// arguments embedded in <format>
   desc: convert fmt/arglist combination into string 
   --------------------------------------------------------------------*/
char *trc_varargs(const char *fmt, ...)
{
	va_list args;
	tthread_t *t = NULL;

	HASH_READ_ENTER();
	if(t = trc_thread(pthread_self())){
		if(fmt){
			va_start(args, fmt);
			vsprintf(t->fmt, fmt, args);
			va_end(args);
		}
		else
			t->fmt[0] = NULL;
	}
	HASH_READ_EXIT();

	return t? t->fmt: NULL;
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_trace(		// procedure name
   taction_t a,		// action
   tlevel_t level,	// trace level
   cchar_t *file, 	// source file 
   uint_t line,		// source file line number 
   cchar_t *kword,	// keyword for trace info
   cchar_t *s)		// trace info
   desc: write trace line 
   --------------------------------------------------------------------*/
void trc_trace(taction_t a, uint_t i, tlevel_t level, cchar_t *file, uint_t line, cchar_t *fn, cchar_t *kword, cchar_t *s)
{
	int n;			/* length of non-indented fields */
	tthread_t *t;
	int num, bytesWritten;
	int unit, thread;
	time_t tim;
	struct tm *timeptr;
	char buf[64];

	HASH_READ_ENTER();
	
	/* check for valid output stream and this thread is registered */
	if(!_fp || !(t = trc_thread(pthread_self()))){
		HASH_READ_EXIT();
		return;
	}

	/* if unit or thread is on and has level set, do trace */
	unit = _unitmax>0 && i>=0 && i<_unitmax && _unit[i].on && ((_unit[i].level & level)!=0); 
	thread = t->on && ((t->level & level)!=0);
	if(!unit && !thread){
		HASH_READ_EXIT();
		return;
	}

	time(&tim);
	timeptr = localtime(&tim);
	strftime(buf, 64, "%D-%T", timeptr);

	/* generate trace output */
	sprintf(t->trace, "%s:%s:%d:%s:%n", buf, file, line, t->name, &n);
	t->wspace[WSPACELEN(n)] = '\0';	

	switch(a){
	case(TRCA_ENTER):
		sprintf(t->trace+n, "%senter %s(%s)\n", t->wspace, fn, s);
		break;
	case(TRCA_RETURN):
		sprintf(t->trace+n, "%sreturn %s(%s)\n", t->wspace, fn, s);
			break;
	case(TRCA_VOID_RETURN):
		sprintf(t->trace+n, "%s%s: return\n", t->wspace, fn);
			break;
	case(TRCA_PRINT):
		sprintf(t->trace+n, "%s%s\n", t->wspace, s);
			break;
	case(TRCA_ERROR):
		sprintf(t->trace+n, "%sERROR in fn %s: %s\n", t->wspace, fn, s);
			break;
	}
	
	t->wspace[WSPACELEN(n)] = ' ';	

	/* write trace output to _fd */
	if(_server){	
		num = 0;
		for(bytesWritten=0; bytesWritten<TRACELEN; bytesWritten+=num)
			num = write(_serv_sockfd, 
				(void*)((char *)t->trace + bytesWritten),
				 TRACELEN - bytesWritten);

		if(bytesWritten == TRACELEN)
			_msgs ++;
	}
	else{
		fprintf(_fp, "%s", t->trace);
	}
	HASH_READ_EXIT();
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_start_client()	// procedure name
   --------------------------------------------------------------------*/
int trc_start_client()
{
	int nread;
	struct sockaddr_in serv_addr = {0};
	char name[1024];
	struct hostent *hostptr = NULL;

	sem_wait(&_startclient);

	/* create endpoint */
	if((_serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror(NULL);
		return 1;
	}

	gethostname(name, 1024);

	hostptr = gethostbyname(name);

	(void) memset(&serv_addr, 0, sizeof(serv_addr));
	(void) memcpy(&serv_addr.sin_addr, hostptr->h_addr, hostptr->h_length);

	/* connect to trace server */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(TRC_PORT);

	if(connect(_serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("connect error\n");
		perror(NULL);
		return 1;
	}	

	return 0;
}

int trc_stop_client()
{
	close(_serv_sockfd);
}

/* ----------------------------------------------------------------------
   void			// procedure 
   trc_start_server()	// procedure name
   --------------------------------------------------------------------*/
void *trc_start_server(void)
{
	char buf[TRACELEN];
	int sockfd, client_sockfd;
	int status = 0, on =1;
	int nread, len;
	struct sockaddr_in serv_addr, client_addr;

	/* create endpoint */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror(NULL);
		return NULL;
	}

	/* turn off bind address checking */
	status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
		(const char *)&on, sizeof(on));
	if(status == -1){
		perror("setsockopt(..., SO_REUSEADDR,...)");
		return NULL;
	}

	/* bind adress */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(TRC_PORT);

	if(bind(sockfd, &serv_addr, sizeof(serv_addr)) < 0){
		perror(NULL);
		return NULL;
	}

	/* specify queue */
	listen(sockfd, 1);

	/* server ready, initialise client */
	sem_post(&_startclient);
	
	len = sizeof(client_addr);
	client_sockfd = accept(sockfd, &client_addr, &len);

	while((_server && (client_sockfd != -1))|| _msgs){
		nread = 0;
		while((nread += read(client_sockfd, buf, TRACELEN)) < TRACELEN)
			;
		_msgs--;
//		printf("%s", buf);
		fprintf(_fp, "%s", buf);
	}
	close(client_sockfd);

	return NULL;
}

int trc_stop_server()
{
	return (_server = 0);
}
