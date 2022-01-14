#include <pthread.h>
#include <ctrace.h>
#include "units.h"
#include "server.h"

int foo(int n)
{
    TRC_ENTER(UNIT_FOO, TRC1, ("n=%d", n));
		
    if(n <= 0){
        TRC_ERROR(UNIT_FOO, ("Invalid argument: %d", n));
        TRC_RETURN(UNIT_FOO, TRC1, ("%d", 1), 1);
    }

    int i;
    for(i=n; i>0; i--){
	sleep(1);
        TRC_PRINT(UNIT_FOO, TRC0, ("Hello world[%d]", i));
    }

    TRC_RETURN(UNIT_FOO, TRC1, ("%d", 0), 0);
}

void *bar(void *arg)
{
    int n = *(int *)arg;

    TRC_ADD_THREAD("bar", 0);
    TRC_ENTER(UNIT_BAR, TRC1, ("arg=0x%x", arg));

    if(n <= 0){
        TRC_ERROR(UNIT_FOO, ("Invalid argument: %d", n));
        TRC_RETURN(UNIT_FOO, TRC1, ("0x%x", NULL), NULL);
    }

    int i;
    for(i=n; i>0; i--){
        sleep(1);
	TRC_PRINT(UNIT_BAR, TRC0, ("Goodbye cruel world[%d]", i));
    }

    TRC_RETURN(UNIT_BAR, TRC1, ("0x%x", NULL), NULL);
}

int main(int argc, char **argv)
{
    int n;
    pthread_t tid;

    if(argc < 2){
        printf("usage: foo <num_msgs>\n");
        exit(1);
    }

    TRC_INIT(NULL, TRC_DISABLED, TRC_OFF, TRC_ALL, UNIT_MAX, 0);
    TRC_ADD_THREAD(argv[0], 0);
    
    server_start();

    n = atoi(argv[1]);
    pthread_create(&tid, NULL, bar, &n);
    foo(n);
    pthread_join(tid, 0);

    server_stop();

    TRC_REMOVE_THREAD(tid);
    TRC_REMOVE_THREAD(pthread_self());
    TRC_END();
}
