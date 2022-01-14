#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include "sched.h"

const int N1 = 2;
const int N2 = 3;

int x = 0;

// F: code blocks annotated with "// F:" are introduced to guide fuzzing
int num_td;     // number of threads concerned
int num_kp;     // number of kep points per thread
int kp[10000];  // how many times should a thread yield before a kp

// this function increases x by 1 in each iteration
void *incr(void *data) {
	while(get_tid_to_run() == 0);		// F: wait until its his turn
    int sched_tid = get_tid_to_run();	// F: acquire the tid for sechduling
    set_tid_to_run(0);					// F: notify main thread to run

    // F: set the thread to use deadline scheduling
    set_sched_dl(1000 * 1000, 1000 * 1000 * 10, 1000 * 1000 * 10);
    int idx_kp = (sched_tid - 1) * num_kp;

    printf("t_%d: started\n", sched_tid);

    for(int i = 0; i < N1; i++) {

        // F: give up several circles' time
        while(1) {
            printf("t_%d: %d times to yield at KP %d.%d\n", sched_tid,
			       kp[idx_kp], sched_tid, idx_kp - (sched_tid - 1) * num_kp);
            if(kp[idx_kp] <= 0) break;
            sched_yield();
            kp[idx_kp]--;
        }
        idx_kp++;

        x += 1;     // a key point
		printf("t_%d: x = %d\n", sched_tid, x);
    }
    printf("t_%d: finished\n", sched_tid);
}

// this function doubles x in each iteration
void *doub(void *data) {
	while(get_tid_to_run() == 0);		// F: wait until its his turn
    int sched_tid = get_tid_to_run();	// F: acquire the tid for sechduling
    set_tid_to_run(0);					// F: notify main thread to run

    // F: set the thread to use deadline scheduling
    set_sched_dl(1000 * 1000, 1000 * 1000 * 10, 1000 * 1000 * 10);
    int idx_kp = (sched_tid - 1) * num_kp;

    printf("t_%d: started\n", sched_tid);

    for(int i = 0; i < N2; i++) {

        // F: give up several circles' time
        while(1) {
			printf("t_%d: %d times to yield at KP %d.%d\n", sched_tid,
			       kp[idx_kp], sched_tid, idx_kp - (sched_tid - 1) * num_kp);
            if(kp[idx_kp] <= 0) break;
            sched_yield();
            kp[idx_kp]--;
        }
        idx_kp++;

        x *= 2;     // a key point
        printf("t_%d: x = %d\n", sched_tid, x);
    }
    printf("t_%d: finished\n", sched_tid);
}

int main(int argc, char *argv[]) {
    // F: set up the yield patten
    num_td = (int) strtol(argv[1], NULL, 10);
    num_kp = (int) strtol(argv[2], NULL, 10);
    for(int i = 0; i < num_td * num_kp; i++)
        kp[i] = (int) strtol(argv[i + 3], NULL, 10);

    // for(int i = 0; i < 20; i++)
    //     printf("%d ", kp[i]);
    // printf("\n");

	set_tid_to_run(0);	// F: allow the main thread to run
	int c_tid = 0;		// F: child tid, initially 0


    pthread_t t1, t2, t_dummy;

	while(get_tid_to_run() != 0);	// F: wait until its his turn
	c_tid++;						// F: get the next child tid
	set_tid_to_run(c_tid);			// F: notify thread 1 to get ready

    pthread_create(&t1, NULL, incr, NULL);

	while(get_tid_to_run() != 0);	// F: wait until its his turn
	c_tid++;						// F: get the next child tid
	set_tid_to_run(c_tid);			// F: notify thread 2 to get ready
	
    pthread_create(&t2, NULL, doub, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // for(int i = 0; i < 20; i++)
    //     printf("%d ", kp[i]);
    // printf("\n");

    printf("x = %d\n", x);

    return 0;
}
