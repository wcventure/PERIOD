#include <pthread.h>
#include <semaphore.h>
#include <ctrace.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "units.h"
#include "server.h"

#define TRACE_LEVEL_MAX	6

#define TRACE_CALL_LEN 	64
#define TRACE_SERVER_PORT 2014

typedef struct service_t{
	pthread_t tid;
	char buf[64];	
	int sockfd;
}service_t;

static int _run = 1;
static pthread_t _tid_main;
static pthread_t _tid_join;
static int _client_sockfd;
static pthread_t _thread_to_join;
static sem_t _sem_a;
static sem_t _sem_b;

void *server_run(void *);
void *server_do_service(void *arg);
void *server_join_threads(void *arg);
void server_do_command(char *buf);

int server_start()
{
	// start a server to respond to trace requests
	if(pthread_create(&_tid_main, NULL, server_run, NULL)){
		printf("Unable to create server thread\n");
		return 1;
	}

	if(pthread_create(&_tid_join, NULL, server_join_threads, NULL)){
		printf("Unable to create server thread\n");
		return 1;
	}

	sem_init(&_sem_a, 0, 1);
	sem_init(&_sem_b, 0, 0);

	return 0;
}


void *server_run(void *arg)
{
	int sockfd;
	int status = 0, on =1;
	int nread, len, num, bytesWritten;
	struct sockaddr_in serv_addr, client_addr;
	int nnread;
	service_t *service = NULL;

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
	serv_addr.sin_port = htons(TRACE_SERVER_PORT);

	if(bind(sockfd, &serv_addr, sizeof(serv_addr)) < 0){
		perror(NULL);
		return NULL;
	}

	/* specify queue */
	listen(sockfd, 1);

	len = sizeof(client_addr);

	while(_run){
		service = (service_t *)malloc(sizeof(service_t));
		service->sockfd = accept(sockfd, &client_addr, &len);
		pthread_create(&(service->tid), NULL, server_do_service, service);
	}

	return NULL;
}


void *server_do_service(void *arg)
{
	service_t *service = (service_t *)arg;
	int nread, nnread, num, bytesWritten;
	
	if(service == NULL) return NULL;

	while(service->sockfd != -1){

		// read next request into buffer
		nread = 0;
		while((nread = recv(service->sockfd, service->buf, TRACE_CALL_LEN, 0)) != -1){
			nnread += nread;
			if(nnread >= TRACE_CALL_LEN)
				break;
			else if(nnread < 0){
				printf("FUCK OFF\n");
				break;
			}
		}

		server_do_command(service->buf);

		for(bytesWritten=0; bytesWritten<64; bytesWritten+=num)
			num = write(service->sockfd, 
				(void*)((char *)service->buf+bytesWritten),
				64 - bytesWritten);
		
		if(bytesWritten != 64 || (!strcmp(service->buf, "Goodbye."))){
			break;
		}
	}

	close(service->sockfd);
	service->sockfd = -1;

	// flag this thread to be joined
	sem_wait(&_sem_a);
	_thread_to_join = service->tid;
	sem_post(&_sem_b);

	return NULL;
}

void *server_join_threads(void *arg)
{
	while(_run){
		sem_wait(&_sem_b);
		pthread_join(_thread_to_join, NULL);
		sem_post(&_sem_a);
	}

	return NULL;	
}

void server_stop()
{
	int num, bytesWritten;
	char buf[64];

	// set running server to stop
	_run = 0;

	memset(buf, 0, sizeof(buf));
	// now pass a dummy request to it in case it is reading from it's port
	for(bytesWritten=0; bytesWritten<64; bytesWritten+=num)
		num = write(_client_sockfd, 
			(void*)((char *)buf+bytesWritten),
			64 - bytesWritten);

	pthread_join(_tid_main, NULL);
	pthread_join(_tid_join, NULL);

	sem_destroy(&_sem_a);
	sem_destroy(&_sem_b);
}

//  client message processing functions 

int server_get_level(char *level)
{
	if(!strcmp(level, "TRC_NONE"))
		return TRC_NONE;
	else if(!strcmp(level, "TRC0"))
		return TRC0;
	else if(!strcmp(level, "TRC1"))
		return TRC1;
	else if(!strcmp(level, "TRC2"))
		return TRC2;
	else if(!strcmp(level, "TRC3"))
		return TRC3;
	else if(!strcmp(level, "TRC4"))
		return TRC4;
	else if(!strcmp(level, "TRC5"))
		return TRC5;
	else if(!strcmp(level, "TRC_ERR"))
		return TRC_ERR;
	else if(!strcmp(level, "TRC_ALL"))
		return TRC_ALL;
	else
		return -1;

}

int server_get_unit(char *unit)
{
	printf("server_get_unit: %s\n", unit);
	if(!strcmp(unit, "UNIT_FOO"))
		return UNIT_FOO;
	else if(!strcmp(unit, "UNIT_BAR"))
		return UNIT_BAR;
	else
		return -1;
}


int server_parse_level(char *buf){
	int i, j, k, level = -1;
	char arg[16];
	int levels[TRACE_LEVEL_MAX];


	memset(levels, -1, sizeof(levels));

	k = 0;	
	for(i=0, j=0; i<strlen(buf)+1; i++){
		if(buf[i] == '|' || buf[i]=='\0'){
			arg[j] = '\0';
			levels[k++] = server_get_level(arg);
			j = 0;
		}
		else
			arg[j++] = buf[i];
	}

	if(levels[0] == -1)
		return -1;

	level = 0;
	for(i=0; i<TRACE_LEVEL_MAX && levels[i]!=-1; i++)
		level |= levels[i];

	return level;
}

void server_do_command(char *buf)
{
	int tid, unit, level;
	char arg1[64], arg2[64], arg3[64];

	sscanf(buf, "%s", arg1);

	if(!strcmp(arg1, "QUIT")){
		sprintf(buf, "Goodbye.");
		return;
	}
	else if(!strcmp(arg1, "TRC_ENABLE"))
		TRC_ENABLE();
	else if(!strcmp(arg1, "TRC_DISABLE"))
		TRC_DISABLE();
	else if(!strcmp(arg1, "TRC_TURN_ON"))
		TRC_TURN_ON();
	else if(!strcmp(arg1, "TRC_TURN_OFF"))
		TRC_TURN_OFF();
	else if(!strcmp(arg1, "TRC_FILE")){
		sscanf(buf, "%s %s", arg1, arg2);
		if(TRC_FILE(arg2)){
			sprintf(buf, "Error: invalid file %s", arg2);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_TURN_THREAD_ON")){
		sscanf(buf, "%s %d", arg1, &tid);
		if(TRC_TURN_THREAD_ON(tid)){
			sprintf(buf, "Error: invalid thread id %d.", tid);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_TURN_THREAD_OFF")){
		sscanf(buf, "%s %d", arg1, &tid);
		if(TRC_TURN_THREAD_OFF(tid)){
			sprintf(buf, "Error: invalid thread id %d.", tid);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_SET_LEVEL")){
		sscanf(buf, "%s %s", arg1, arg2);
		if((level = server_parse_level(arg2)) == -1 || TRC_SET_LEVEL(level)){
			sprintf(buf, "Error: invalid level %d.", level);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_ADD_LEVEL")){
		sscanf(buf, "%s %s", arg1, arg2);
		if((level = server_parse_level(arg2)) == -1 || TRC_ADD_LEVEL(level)){
			sprintf(buf, "Error: invalid level %d.", level);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_REMOVE_LEVEL")){
		sscanf(buf, "%s %s", arg1, arg2);
		if((level = server_parse_level(arg2)) == -1 || TRC_REMOVE_LEVEL(level)){
			sprintf(buf, "Error: invalid level %d.", level);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_TURN_UNIT_ON")){
		sscanf(buf, "%s %s", arg1, arg2);
		if((unit = server_get_unit(arg2)) == -1 || TRC_TURN_UNIT_ON(unit)){
			sprintf(buf, "Error: invalid unit %d.", unit); 
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_TURN_UNIT_OFF")){
		sscanf(buf, "%s %s", arg1, arg2);
		if((unit = server_get_unit(arg2)) == -1 || TRC_TURN_UNIT_OFF(unit)){
			sprintf(buf, "Erorr: invalid unit %d.", unit);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_SET_UNIT_LEVEL")){
		sscanf(buf, "%s %s %s", arg1, arg2, arg3);
		if((unit = server_get_unit(arg2)) == -1 || (level = server_parse_level(arg3)) == -1 || TRC_SET_UNIT_LEVEL(unit, level)){
			sprintf(buf, "Error: invalid unit %d or invalid level %d.", unit, level);
			return;
		}
	}
	else if(!strcmp(arg1, "TRC_ADD_UNIT_LEVEL")){
		sscanf(buf, "%s %s %s", arg1, arg2, arg3);
		if((unit = server_get_unit(arg2)) == -1 || (level = server_parse_level(arg3)) == -1 || TRC_ADD_UNIT_LEVEL(unit, level)){
			sprintf(buf, "Error: invalid unit %d or invalid level %d.", unit, level);
			return;
		}	
	}
	else if(!strcmp(arg1, "TRC_REMOVE_UNIT_LEVEL")){
		sscanf(buf, "%s %s %s", arg1, arg2, arg3);
		if((unit = server_get_unit(arg2)) == -1 || (level = server_parse_level(arg3)) == -1 || TRC_REMOVE_UNIT_LEVEL(unit, level)){
			sprintf(buf, "Error: invalid unit %d or invalid level %d.", unit, level);
			return;
		}
	}
	else{
		sprintf(buf, "Error: unknown command: %s", arg1);
		return;
	}

	sprintf(buf, "Done.");
}
