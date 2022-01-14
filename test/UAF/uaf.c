#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int ret;
char buf[7];
char* ptr1;
char* ptr2;

void *t1(void * data)
{
	if(buf[6] < 'm')
		ret = 1;
	if(buf[5] == 'e')
		ptr2 = ptr1;
	if(buf[3] == 's')
		if(buf[1] == 'u'){
			free(ptr1);
			printf("free\n");
		}
}

void *t2(void *data)
{
	if(buf[4] == 'e')
		if(buf[2] == 'r')
			if(buf[0] == 'f'){
				ptr2[0] = 'm';
				printf("use\n");
			}
}

int main(int argc, char **argv) {
	printf("start\n");
	ret = 0;
	ptr1 = (char*)malloc(7);
	ptr2 = (char*)malloc(7);
	//if (read(0, buf, 7) < 1)
	   // exit(1);
	buf[0]='f';
	buf[1]='u';
	buf[2]='r';
	buf[3]='s';
	buf[4]='e';
	buf[5]='e';
	buf[6]='a';
	pthread_t id1, id2;
	pthread_create(&id2, NULL, t2, NULL);
	pthread_create(&id1, NULL, t1, NULL);
	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	printf("finished\n");
	
	return ret;
}
