#include <pthread.h>
#include <stdio.h>

int x = 0;

// this function increases x by 1 in each iteration
void *incr(void *data) {
    for(int i = 0; i < 2; i++) {
        x += 1;     // key point 0
    }
    printf("finished 1\n");
    return NULL;
}

// this function doubles x in each iteration
void *doub(void *data) {
    for(int i = 0; i < 3; i++) {
        x *= 2;     // key point 1
    }
    printf("finished 2\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, incr, NULL);
	pthread_create(&t2, NULL, doub, NULL);
    pthread_join(t2, NULL);
	pthread_join(t1, NULL);
	
    printf("x = %d\n", x);
    return 0;
}
