#include <pthread.h>
#include <stdio.h>
#include "sched.h"

const int N1 = 2;
const int N2 = 3;

int x = 0;

// F: code blocks annotated with "// F:" are introduced to guide fuzzing
int num_kp = 2; // number of key points
int kp[2];      // a thread should sleep kp[i] nanoseconds
                // before it reaches the i_th key point

// this function increases x by 1 in each iteration
void *incr(void *data) {
    // F: set the thread to use deadline scheduling
    set_sched_dl(1024, 1024 * 15, 1024 * 15);

    for(int i = 0; i < N1; i++) {
        // F: give up a circle's time
        if(kp[0] % 2) {
            //printf("yield 1\n");
            sched_yield();
            //kp[0]--;
        }
        kp[0] /= 2;

        x += 1;     // key point 0
        printf("%d\n", x);
    }
    printf("finished 1\n");
}

// this function doubles x in each iteration
void *doub(void *data) {
    // F: set the thread to use deadline scheduling
    set_sched_dl(1024, 1024 * 15, 1024 * 15);

    for(int i = 0; i < N2; i++) {
        // F: sleep kp[1] nanoseconds before getting to the key point 1
        if(kp[1] % 2) {
            //printf("yield 2\n");
            sched_yield();
            //kp[1]--;
        }
        kp[1] /= 2;

        x *= 2;     // key point 1
        printf("%d\n", x);
    }
    printf("finished 2\n");
}

int main(int argc, char *argv[]) {
    // F: set up sleep patten for this round's execution
    for(int i = 0; i < num_kp; i++) kp[i] = (int) strtol(argv[i + 1], NULL, 10);
    printf("kp: %d, %d\n", kp[0], kp[1]);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, incr, NULL);
    pthread_create(&t2, NULL, doub, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("x = %d\n", x);

    return 0;
}
