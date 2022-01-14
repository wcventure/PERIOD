#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

void* g;

void work() {
  g = malloc(10);
  if (g != NULL) {
    *(int*)g = 1;
    free(g);
  }
  g = NULL;
}

void run() {
  for (int i=0; i<1; i++) {
    work();
  }
}

void main() {
  pthread_t tid1, tid2;
  pthread_create(&tid1, NULL, (void*)run, NULL);
  pthread_create(&tid2, NULL, (void*)run, NULL);
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
}

