#include <pthread.h>
pthread_mutex_t  m, l;
int A = 0, B = 0;

void *t1(void *arg) {
	pthread_mutex_lock(&m);
	A++;
	if (A == 1) pthread_mutex_lock(&l);
	pthread_mutex_unlock(&m);
	//perform class A operation
	pthread_mutex_lock(&m);
	A--;
	if (A == 0) pthread_mutex_unlock(&l);
	pthread_mutex_unlock(&m);
	return NULL;
}

void *t2(void *arg) {
	pthread_mutex_lock(&m);
	B++;
	if (B == 1) pthread_mutex_lock(&l);
	pthread_mutex_unlock(&m);
	//perform class B operation
	pthread_mutex_lock(&m);
	B--;
	if (B == 0) pthread_mutex_unlock(&l);
	pthread_mutex_unlock(&m);
	return NULL;
}

int main(void) {
	pthread_mutex_init(&m,NULL);
	pthread_mutex_init(&l,NULL);
	pthread_t T1, T2;
	pthread_create(&T1, NULL, t1, NULL); 
	pthread_create(&T2, NULL, t2, NULL);
	pthread_join(T2, NULL);
	pthread_join(T1, NULL);
	return 0;
}
