#ifndef _COM_TRACE__H
#define _COM_TRACE__H

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

#define _STDOUT		stdout
#define _STDERR		stderr
#define _STDIN		stdin

typedef unsigned int uint_t;
typedef const char cchar_t;
typedef pthread_t tid_t;

/* --------------------------------------------------------------- */
/* tracing enabled flag typedef/values */
#define TRC_DISABLED	0
#define TRC_ENABLED	1

typedef uint_t tenable_t;
/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* tracing turned on flag typedef/values */
#define TRC_OFF		0
#define TRC_ON		1

typedef uint_t ton_t;
/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* tracing levels typedef/values */
#define TRC_NONE	0
#define TRC0		1	
#define TRC1		2	
#define TRC2		4
#define TRC3		8
#define TRC4		16
#define TRC5		32
#define TRC_ERR		64	
#define TRC_ALL		TRC0|TRC1|TRC2|TRC3|TRC4|TRC5|TRC_ERR

typedef uint_t tlevel_t;
/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* separate trace thread flag typedef/values */
#define TRC_SERV_OFF	0
#define TRC_SERV_ON	1

typedef uint_t tserver_t;
/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* tracing actions typedef/values */
#define TRCA_ENTER		0	
#define TRCA_RETURN		1	
#define TRCA_VOID_RETURN	2	
#define TRCA_PRINT		3	
#define TRCA_ERROR		4	

typedef uint_t taction_t;
/* --------------------------------------------------------------- */

extern tenable_t _trc; 
static tlevel_t _tlevel;

extern int trc_init(cchar_t *file, tenable_t trc, ton_t on, tlevel_t level, uint_t umax, tserver_t server);
extern void trc_end();
extern int trc_file(cchar_t *file);
extern int trc_add_thread(cchar_t *tname, tid_t id);
extern int trc_remove_thread(tid_t id);
extern void trc_turn_on();
extern void trc_turn_off();
extern int trc_turn_thread_on(tid_t id);
extern int trc_turn_thread_off(tid_t id);
extern int trc_turn_unit_on(uint_t i);
extern int trc_turn_unit_off(uint_t i);
extern int trc_set_level(tlevel_t l);
extern int trc_add_level(tlevel_t l);
extern int trc_remove_level(tlevel_t l);
extern int trc_set_thread_level(tlevel_t l, tid_t id);
extern int trc_add_thread_level(tlevel_t l, tid_t id);
extern int trc_remove_thread_level(tlevel_t l, tid_t id);
extern int trc_set_unit_level(uint_t i, tlevel_t level);
extern int trc_add_unit_level(uint_t i, tlevel_t level);
extern int trc_remove_unit_level(uint_t i, tlevel_t level);
extern char *trc_varargs(const char  *fmt, ...);
extern void trc_trace(taction_t a, uint_t i, tlevel_t tlevel, cchar_t *file,uint_t line, cchar_t *fn, cchar_t *kword, cchar_t *s);
extern void trc_state();
extern void trc_print_threads();


/* ----------------------------------------------------------------------
   TRACE MACROS 
   --------------------------------------------------------------------*/

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_INIT_DEFAULT()	// macro name
   desc: initialise trace library with default values
   --------------------------------------------------------------------*/
#define TRC_INIT_DEFAULT() trc_init(_STDOUT, TRC_DISABLED, TRC_ON, TRC_ERR,  100, 0)

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_INIT(		// macro name
   cchar_t *a,		// output stream(file, stdout, stderr), NULL=stdout  
   tenable_t b,		// trace enabled flag
   ton_t c,		// trace on flag
   tlevel_t d,		// trace levels to default to
   uint_t e,		// maximum number of logical software units
   tserver_t f)		// if server=1 run trace as separate server(thread)
   desc: initialise trace library
   --------------------------------------------------------------------*/
#define TRC_INIT(a,b,c,d,e,f) trc_init(a, b, c, d, e, f)


/* ----------------------------------------------------------------------
   void			// 0=success, 1=failure
   TRC_END()		// macro name
   desc: cleanup trace library. must be called by final sole running thread
   --------------------------------------------------------------------*/
#define TRC_END() trc_end()


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_FILE(		// macro name
   cchar_t *a)		// output stream(file, stdout, stderr), NULL=stdout  
   desc: new output stream for trace library becomes <a>
   --------------------------------------------------------------------*/
#define TRC_FILE(a) trc_file(a)


/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_ENABLE()		// procedure name
   desc: enable tracing 
   --------------------------------------------------------------------*/
#define TRC_ENABLE() \
	_trc = TRC_ENABLED


/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_DISABLE()	// procedure name
   desc: disable tracing 
   --------------------------------------------------------------------*/
#define TRC_DISABLE() \
	_trc = TRC_DISABLED


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_ADD_THREAD(	// macro name
   cchar_t *a,		// trace output thread name 
   tid_t b)		// thread id
   desc: add thread with display name <a> and id <b> to trace library
   if(b == 0) thread defaults to the caller of this macro
   --------------------------------------------------------------------*/
#define TRC_ADD_THREAD(a,b) trc_add_thread(a,b)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_REMOVE_THREAD(	// macro name
   tid_t a)		// thread id
   desc: remove thread with id <a> from trace library
   if(a == 0) thread defaults to the caller of this macro
   --------------------------------------------------------------------*/
#define TRC_REMOVE_THREAD(a) trc_remove_thread(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_TURN_ON()	// macro name
   desc: turn tracing on for all threads 
   --------------------------------------------------------------------*/
#define TRC_TURN_ON() trc_turn_on()


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_TURN_OFF()	// macro name
   desc: turn tracing off for all threads 
   --------------------------------------------------------------------*/
#define TRC_TURN_OFF() trc_turn_off()


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_TURN_THREAD_ON(	// macro name
   tid_t a)		// thread id
   desc: turn tracing on for thread with id <a> 
   if(a == 0) thread defaults to the caller of this macro
   --------------------------------------------------------------------*/
#define TRC_TURN_THREAD_ON(a) trc_turn_thread_on(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_TURN_THREAD_OFF(	// macro name
   tid_t a)		// thread id
   desc: turn tracing off for thread with id <a> 
   if(a == 0) thread defaults to the caller of this macro
   --------------------------------------------------------------------*/
#define TRC_TURN_THREAD_OFF(a) trc_turn_thread_off(a)

/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_TURN_UNIT_ON(	// macro name
   tunit_t a)		// tunit_t 
   desc: turn tracing on for tunit_t <a> 
   --------------------------------------------------------------------*/
#define TRC_TURN_UNIT_ON(a) trc_turn_unit_on(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_TURN_UNIT_OFF(	// macro name
   tunit_t a)		// tunit_t 
   desc: turn tracing off for tunit_t <a> 
   --------------------------------------------------------------------*/
#define TRC_TURN_UNIT_OFF(a) trc_turn_unit_off(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_SET_LEVEL(	// macro name
   tlevel_t a)		// trace level(s)
   desc: set level=<a> in all added threads 
   --------------------------------------------------------------------*/
#define TRC_SET_LEVEL(a) trc_set_level(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_ADD_LEVEL(	// function name
   tlevel_t a)		// trace level(s)
   desc: add level <a> to all added threads 
   --------------------------------------------------------------------*/
#define TRC_ADD_LEVEL(a) trc_add_level(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_REMOVE_LEVEL(	// function name
   tlevel_t a)		// trace level(s)
   desc: remove level <a> from all added threads 
   --------------------------------------------------------------------*/
#define TRC_REMOVE_LEVEL(a) trc_remove_level(a)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_SET_THREAD_LEVEL(// function name
   tlevel_t a		// trace level(s)
   tid_t b)		// thread id
   desc: set level=<a> in thread <b>
   --------------------------------------------------------------------*/
#define TRC_SET_THREAD_LEVEL(a, b) trc_set_thread_level(a, b)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_ADD_THREAD_LEVEL(	// function name
   tlevel_t a		// trace level(s)
   tid_t b)		// thread id
   desc: add level <a> to thread <b>
   --------------------------------------------------------------------*/
#define TRC_ADD_THREAD_LEVEL(a, b) trc_add_thread_level(a, b)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_REMOVE_THREAD_LEVEL(	// function name
   tlevel_t a		// trace level(s)
   tid_t b)		// thread id
   desc: remove level <a> from thread <b>
   --------------------------------------------------------------------*/
#define TRC_REMOVE_THREAD_LEVEL(a, b) trc_remove_thread_level(a, b)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_SET_UNIT_LEVEL(// function name
   tunit_t a		// tunit_t
   tlevel_t b)		// trace level(s) 
   desc: set level=<b> in tunit_t <a>
   --------------------------------------------------------------------*/
#define TRC_SET_UNIT_LEVEL(a, b) trc_set_unit_level(a, b)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_ADD_UNIT_LEVEL(	// function name
   tunit_t a		// trace unit_t
   tlevel_t b)		// trace level(s) 
   desc: add level <b> to tunit_t <a>
   --------------------------------------------------------------------*/
#define TRC_ADD_UNIT_LEVEL(a, b) trc_add_unit_level(a, b)


/* ----------------------------------------------------------------------
   int			// 0=success, 1=failure
   TRC_REMOVE_UNIT_LEVEL(	// function name
   tunit_t a		// trace unit_t
   tlevel_t b)		// trace level(s) 
   desc: remove level <b> from tunit_t <a>
   --------------------------------------------------------------------*/
#define TRC_REMOVE_UNIT_LEVEL(a, b) trc_remove_unit_level(a, b)



/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_ENTER(		// macro name
   tunit_t a,		// unit trace flag for enabling unit trace 
   tlevel_t b,		// trace level
   cchar_t *c)		// formatted vararglist of fn argument values 
   desc: write trace enter function line 
   argument d takes form of ("%d, %s\n", arg1, arg2)
   eg: TRC_ENTER(tunit, TRC0, "main", ("%d, %s", 1, "string"));
   --------------------------------------------------------------------*/
#define TRC_ENTER(a,b,c) \
	if(_trc) \
		trc_trace(TRCA_ENTER, a, b, __FILE__, __LINE__, __FUNCTION__, NULL, trc_varargs c)

/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_RETURN(		// macro name
   tunit_t a,		// unit trace flag for enabling unit trace 
   tlevel_t b,		// trace level
   cchar_t *c,		// formatted arglist of fn return value 
   void *d)		// return value 
   desc: write trace return from function line 
   this macro also performs function return
   argument d takes form of ("%d\n", arg)
   eg: TRC_RETURN(tunit, TRC0, NULL, ("0x%x", NULL)); 
   --------------------------------------------------------------------*/
#define TRC_RETURN(a,b,c,d){ \
	if(_trc) \
		trc_trace(TRCA_RETURN, a, b, __FILE__, __LINE__, __FUNCTION__, NULL, trc_varargs c); \
   	return(d); \
}


/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_VOID_RETURN(	// macro name
   tunit_t a,		// unit trace flag for enabling unit trace 
   tlevel_t b)		// trace level
   desc: write trace return procedure line 
   this macro also performs procedure return
   eg: TRC_VOID_RETURN(tunit, TRC0)
   --------------------------------------------------------------------*/
#define TRC_VOID_RETURN(a,b){ \
	if(_trc) \
		trc_trace(TRCA_VOID_RETURN, a, b, __FILE__, __LINE__, __FUNCTION__, NULL, NULL); \
	return; \
}


/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_PRINT(		// macro name
   tunit_t a,		// unit trace flag for enabling unit trace 
   tlevel_t b,		// trace level
   cchar_t *c)		// formatted arglist  
   desc: write trace print line 
   argument c takes form of ("val=%d, string=%s\n", arg1, arg2)
   eg: TRC_PRINT(tunit, TRC0, ("loopvals: val=%d, string=%s", i, str)); 
   --------------------------------------------------------------------*/
#define TRC_PRINT(a,b,c) \
	if(_trc) \
		trc_trace(TRCA_PRINT, a, b, __FILE__, __LINE__, __FUNCTION__, NULL,trc_varargs c)


/* ----------------------------------------------------------------------
   void			// procedure 
   TRC_ERROR(		// macro name
   tunit_t a,		// unit trace flag for enabling unit trace 
   cchar_t *b)		// formatted arglist  
   desc: write trace error line 
   argument d takes form of ("val=%d, string=%s\n", arg1, arg2)
   eg: TRC_ERROR(tunit, TRC0, "loopvals", ("val=%d, string=%s", i, str)); 
   --------------------------------------------------------------------*/
#define TRC_ERROR(a,b) \
	if(_trc) \
		trc_trace(TRCA_ERROR, a, TRC_ERR, __FILE__, __LINE__, __FUNCTION__, NULL, trc_varargs b)


/* debugging purposes only */
#define TRC_PRINT_THREADS() trc_print_threads()
#define TRC_STATE() trc_state()


#endif		/* _COM_TRACE__H */
