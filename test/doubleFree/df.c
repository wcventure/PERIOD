#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

char* ptr;

void *t1()
{
	free(ptr);
	printf("free 1\n");
	ptr=NULL;
	printf("NULL 1\n");
	
}

void *t2()
{
	free(ptr);
	printf("free 2\n");
	ptr=NULL;
	printf("NULL 2\n");
}

int main(int argc, char **argv) {
	printf("start\n");
	ptr = (char*)malloc(7);
	ptr[0]='a';
	pthread_t id1, id2;
	pthread_create(&id1, NULL, t1, NULL);
	pthread_create(&id2, NULL, t2, NULL);
	pthread_join(id2, NULL);
	pthread_join(id1, NULL);
	printf("finished\n");
	
	return 0;
}
