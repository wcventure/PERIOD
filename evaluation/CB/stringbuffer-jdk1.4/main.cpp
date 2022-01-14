// This file is used to mimic the StringBuffer bug in JDK1.4
// Author: Jie Yu (jieyu@umich.edu)
// Modified: wcventure

#include <stdio.h>
#include <pthread.h>
#include "stringbuffer.hpp"

StringBuffer *buffer = new StringBuffer("abc");

void *thread_main1(void *args) {

	buffer->erase(0, 3);
	buffer->append("abc");
  
	return NULL;
}

void *thread_main2(void *args) {

	StringBuffer *sb = new StringBuffer();
	sb->append(buffer);

	return NULL;
}

int main(int argc, char *argv[]) {

	pthread_t thd1, thd2;
	int rc1;
	int rc2;
	rc1 = pthread_create(&thd1, NULL, thread_main1, NULL);
	rc2 = pthread_create(&thd2, NULL, thread_main2, NULL);
	pthread_join(thd1, NULL);
	pthread_join(thd2, NULL);
  
	return 0;
}
