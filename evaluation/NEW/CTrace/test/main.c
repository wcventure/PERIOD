#include <pthread.h>
#include <semaphore.h>
#include <ctrace.h>

void *thread1(void *);
void *thread2(void *);

sem_t sem;

void myprocedure()
{
	TRC_ENTER(0, TRC0, NULL);
	TRC_VOID_RETURN(0, TRC0);
}

int main(int argc, char **argv)
{
	int i = 6;
	tid_t tid1, tid2;
	

	sem_init(&sem, 0, 1);

	TRC_INIT(NULL, TRC_ENABLED, TRC_ON, TRC0, 0, TRC_SERV_OFF);
	TRC_ADD_THREAD(argv[0], 0);
	TRC_ENTER(0, TRC0, ("%d, %s", 1, "string"));


	TRC_ADD_LEVEL(TRC2);

	if(pthread_create(&tid1, NULL, thread1, NULL))
		return 1;

	if(pthread_create(&tid2, NULL, thread2, NULL))
		return 1;

	sem_post(&sem);
	sem_post(&sem);

	myprocedure();

	TRC_PRINT(0, TRC0, ("i=%d", 0));
	TRC_ERROR(0,  ("mystring=%s", "stringval"));
	


	pthread_join(tid1, NULL);
	TRC_REMOVE_THREAD(tid1);
	pthread_join(tid2, NULL);
	TRC_REMOVE_THREAD(tid2);
	TRC_RETURN(0, TRC0, ("%d", i), i);
	TRC_END();

	sem_destroy(&sem);
}

void *thread1(void *arg)
{
	TRC_ADD_THREAD("thread1", 0);
	TRC_SET_THREAD_LEVEL(TRC4, 0);
	TRC_ADD_THREAD_LEVEL(TRC0, 0);
	TRC_REMOVE_THREAD_LEVEL(TRC4, 0);
	TRC_TURN_THREAD_ON(0);
	sem_wait(&sem);
	TRC_ENTER(0, TRC0, ("%d", 1));
	TRC_PRINT(0, TRC0, ("level0: i=%d", 1));
	TRC_PRINT(0, TRC1, ("level1: i=%d", 1));
	TRC_PRINT(0, TRC2, ("level2: i=%d", 1));
	TRC_PRINT(0, TRC3, ("level3: i=%d", 1));
	TRC_PRINT(0, TRC4, ("level4: i=%d", 1));

	TRC_RETURN(0, TRC0, ("0x%x", NULL), NULL);
}

void *thread2(void *arg)
{
	TRC_ADD_THREAD("thread2", 0);
	sem_wait(&sem);
	TRC_ENTER(0, TRC0, ("%d", 2));
	TRC_PRINT(0, TRC0, ("level0: i=%d", 2));
	TRC_PRINT(0, TRC1, ("level1: i=%d", 2));
	TRC_PRINT(0, TRC2, ("level2: i=%d", 2));
	TRC_PRINT(0, TRC3, ("level3: i=%d", 2));
	TRC_PRINT(0, TRC4, ("level4: i=%d", 2));
	TRC_RETURN(0, TRC0, ("0x%x", NULL), NULL);
}
