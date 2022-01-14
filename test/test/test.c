#include <pthread.h>
#include <assert.h>
static int done = 0;
static int worker = 0;
pthread_mutex_t  m;

void *Once (void *args) {
	pthread_mutex_lock(&m);
	if (done)
		return 0;
	pthread_mutex_unlock(&m);

	worker++;
	
	pthread_mutex_lock(&m);
	if (!done) 
		done = 1;
	pthread_mutex_unlock(&m);
}
int main(int argc, char **argv) {
	pthread_t id1, id2;
	pthread_create(&id2, NULL, Once, NULL);
	pthread_create(&id1, NULL, Once, NULL);
	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	assert(worker==1);
	return 0;
}
