#include <pthread.h>
#include <stdio.h>
#include <sys/shm.h>
#include "sched.h"

const int N1 = 2;
const int N2 = 3;

int x = 0;

// F: code blocks annotated with "// F:" are introduced to guide fuzzing
typedef struct Sched_Info {
    int N;          // number of threads concerned
    int M;          // number of kep points per thread
    int kp[10000];  // how many times should a thread yield before a kp
} sched_info;

// F: the information used to guide the scheduling
sched_info * si;

// this function increases x by 1 in each iteration
void *incr(void *data) {

	while(get_tid_to_run() == 0){}		// F: wait until its his turn
    int sched_tid = get_tid_to_run();	// F: acquire the tid for sechduling
    set_tid_to_run(0);					// F: notify main thread to run

    // F: set the thread to use deadline scheduling
    set_sched_dl(1000 * 1000, 1000 * 1000 * 10, 1000 * 1000 * 10);
    int idx_kp = (sched_tid - 1) * (si -> M);

    printf("t_%d: started\n", sched_tid);

    for(int i = 0; i < N1; i++) {

        // F: give up several circles' time
        while(1) {
            printf("t_%d: %d times to yield at KP %d.%d\n", sched_tid,
				   si -> kp[idx_kp], sched_tid,
				   idx_kp - (sched_tid - 1) * (si -> M));
            if(si -> kp[idx_kp] <= 0) break;
            sched_yield();
            si -> kp[idx_kp]--;
        }
        idx_kp++;

        x += 1;     // a key point
		printf("t_%d: x = %d\n", sched_tid, x);
    }
    printf("t_%d: finished\n", sched_tid);

    return NULL;
}

// this function doubles x in each iteration
void *doub(void *data) {
	while(get_tid_to_run() == 0){}		// F: wait until its his turn
    int sched_tid = get_tid_to_run();	// F: acquire the tid for sechduling
    set_tid_to_run(0);					// F: notify main thread to run

    // F: set the thread to use deadline scheduling
    set_sched_dl(1000 * 1000, 1000 * 1000 * 10, 1000 * 1000 * 10);
    int idx_kp = (sched_tid - 1) * (si -> M);

    printf("t_%d: started\n", sched_tid);

    for(int i = 0; i < N2; i++) {

        // F: give up several circles' time
        while(1) {
			printf("t_%d: %d times to yield at KP %d.%d\n", sched_tid,
				   si -> kp[idx_kp], sched_tid,
				   idx_kp - (sched_tid - 1) * (si -> M));
            if(si -> kp[idx_kp] <= 0) break;
            sched_yield();
            si -> kp[idx_kp]--;
        }
        idx_kp++;

        x *= 2;     // a key point
        printf("t_%d: x = %d\n", sched_tid, x);
    }
    printf("t_%d: finished\n", sched_tid);

    return NULL;
}

int main(int argc, char *argv[]) {
    // F: set up the yield patten using shared memory
	int shmid;
    key_t key = 1111;
    // Setup shared memory
    if ((shmid = shmget(key, sizeof(sched_info), IPC_CREAT | 0666)) < 0) {
        printf("Error getting shared memory id. 1\n");
        exit(1);
    }
	// Attached shared memory
    if ((sched_info *) -1 == (si = (sched_info *) shmat(shmid, NULL, 0))) {
        printf("Error attaching shared memory id. 2\n");
        exit(1);
    }

    // for(int i = 0; i < 20; i++)
    //     printf("%d ", si -> kp[i]);
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

    printf("x = %d\n", x);

    return 0;
}
