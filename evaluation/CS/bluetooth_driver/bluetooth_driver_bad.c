#include <pthread.h>
#include <assert.h>
#include "common.inc"

#define TRUE  (1)
#define FALSE (0)

//void __ESBMC_atomic_begin();
//void __ESBMC_atomic_end();

typedef struct DEVICE {
  int pendingIo;
  _Bool stoppingFlag;
  _Bool stoppingEvent;
} DEVICE_EXTENSION;

_Bool stopped;

int BCSP_IoIncrement(DEVICE_EXTENSION *e)
{
  if (e->stoppingFlag)
    return -1;

  __ESBMC_atomic_begin();
  e->pendingIo = e->pendingIo + 1;
  __ESBMC_atomic_end();  
  
  return 0;
}

void BCSP_IoDecrement(DEVICE_EXTENSION *e)
{
  int pendingIo;

  __ESBMC_atomic_begin();
  e->pendingIo = e->pendingIo - 1;
  pendingIo = e->pendingIo;
  __ESBMC_atomic_end();  

  if (pendingIo == 0)
    e->stoppingEvent = TRUE;
}

void* BCSP_PnpAdd(DEVICE_EXTENSION *e)
{
  int status;

  status = BCSP_IoIncrement(e);
  if (status == 0)
  {
    //do work here
    assert(!stopped);
  }
  BCSP_IoDecrement(e);
  return NULL;
}

void* BCSP_PnpStop(void* arg)
{
  DEVICE_EXTENSION *e;
  e = (DEVICE_EXTENSION *) arg; 

  e->stoppingFlag = TRUE;
  BCSP_IoDecrement(e);
  if(e->stoppingEvent)
  {
    //release allocated resource
    stopped = TRUE;
  }
  return NULL;
}

void main() 
{
  pthread_t id1, id2;
  DEVICE_EXTENSION e;

  e.pendingIo = 1;
  e.stoppingFlag = FALSE;
  e.stoppingEvent = FALSE;
  stopped = FALSE;

  pthread_create(&id1, NULL, BCSP_PnpStop, &e);
  pthread_create(&id2, NULL, BCSP_PnpAdd, &e);
  pthread_join(id2, NULL);
  pthread_join(id1, NULL);
}
