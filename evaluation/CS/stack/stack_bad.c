#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#define TRUE	  (1)
#define FALSE	  (0) 
#define SIZE	  (10)
#define OVERFLOW  (-1)
#define UNDERFLOW (-2)

//unsigned int nondet_uint();
static int top=0;
static unsigned int arr[SIZE];
pthread_mutex_t m;
_Bool flag=FALSE;

void inc_top(void)
{
  top++;
}

void dec_top(void)
{
  top--;
}

int get_top(void)
{
  return top;
}

int stack_empty(void)
{
  (top==0) ? TRUE : FALSE; 
}

int push(unsigned int *stack, int x)
{
  if (top==SIZE) 
  {
    printf("stack overflow\n");
    return OVERFLOW;
  } 
  else 
  {
    stack[get_top()] = x;
    inc_top();
  }
  return 0;
}

int pop(unsigned int *stack)
{
  if (get_top()==0) 
  {
    printf("stack underflow\n");	
    return UNDERFLOW;
  } 
  else 
  {
    dec_top();
    return stack[get_top()];  
  }
  return 0;
}

void *t1(void *arg) 
{
  int i;

  for(i=0; i<SIZE; i++)
  {
    pthread_mutex_lock(&m);
    assert(push(arr,i)!=OVERFLOW);
    flag=TRUE;
    pthread_mutex_unlock(&m);
  }
  return NULL;
}

void *t2(void *arg) 
{
  int i;

  for(i=0; i<SIZE; i++)
  {
    pthread_mutex_lock(&m);
    if (flag)
      assert(pop(arr)!=UNDERFLOW); /* BAD */
    pthread_mutex_unlock(&m);
  }
  return NULL;
}


int main(void) 
{
  pthread_t id1, id2;

  pthread_mutex_init(&m, 0);

  pthread_create(&id1, NULL, t1, NULL);
  pthread_create(&id2, NULL, t2, NULL);

  pthread_join(id1, NULL);
  pthread_join(id2, NULL);

  return 0;
}
