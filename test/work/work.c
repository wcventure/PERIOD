#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

char *g;

void *Work(void *args) {

	g = (char*)malloc(10); printf("%p\n",g);
	if (g != NULL) {	
		free(g);
	}
	g = NULL;
}

int main(int argc, char **argv) {
	pthread_t id1, id2;
	pthread_create(&id2, NULL, Work, NULL);
	pthread_create(&id1, NULL, Work, NULL);
	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	printf("finished\n");
	return 0;
}
