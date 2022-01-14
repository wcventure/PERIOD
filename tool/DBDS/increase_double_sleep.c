#include <pthread.h>
#include <stdio.h>
#include "sched.h"

const int N1 = 3;
const int N2 = 3;

int x = 0;

// F: code blocks annotated with "// F:" are introduced to guide fuzzing
int num_kp = 2; // number of key points
int kp[2];      // a thread should sleep kp[i] nanoseconds
                // before it reaches the i_th key point

// this function increases x by 1 in each iteration
void *incr(void *data) {
    for(int i = 0; i < N1; i++) {
        // F: sleep kp[0] nanoseconds before getting to the key point 0
        usleep(kp[0]);

        x += 1;     // key point 0
    }
}

// this function doubles x in each iteration
void *doub(void *data) {
    for(int i = 0; i < N2; i++) {
        // F: sleep kp[1] nanoseconds before getting to the key point 1
        usleep(kp[1]);

        x *= 2;     // key point 1
    }
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
