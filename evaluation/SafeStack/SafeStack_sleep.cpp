
#include <stdio.h>
#include <atomic>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

template<typename T>
struct SafeStackItem
{
  volatile T Value;
  std::atomic<int> Next;
};

template<typename T>
class SafeStack
{
  std::atomic<int> head;
  std::atomic<int> count;

public:
  SafeStackItem<T>* array;

  SafeStack(int pushCount)
  {
      array = new SafeStackItem<T> [pushCount];
      count.store(pushCount, std::memory_order_relaxed);
      head.store(0, std::memory_order_relaxed);
      for (int i = 0; i < pushCount - 1; i++)
          array[i].Next.store(i + 1, std::memory_order_relaxed);
      array[pushCount - 1].Next.store(-1, std::memory_order_relaxed);
  }

  ~SafeStack()
  {
      delete [] array;
  }
  int getHead()
  {
	  return head;
  }
  int getCount()
  {
	  return count;
  }

  int Pop0()
  {
	  while (count.load(std::memory_order_acquire) > 1)
      {
          int head1 = head.load(std::memory_order_acquire);
		  printf("thread 0: reads head = %d in Pop()\n", head1);
          int next1 = array[head1].Next.exchange(-1, std::memory_order_seq_cst);
		  printf("thread 0: executes Exchange on the Next field of the element %d, he reads %d and writes -1\n", head1, next1);

          if (next1 >= 0)
          {
              int head2 = head1;
			  if (head.compare_exchange_strong(head2, next1, std::memory_order_seq_cst))
              {
                  count.fetch_sub(1, std::memory_order_seq_cst);
                  return head1;
              }
              else
              {
                  array[head1].Next.exchange(next1, std::memory_order_seq_cst);
              }
          }
      }

      return -1;
  }
  
  int Pop1()
  {
      while (count.load(std::memory_order_acquire) > 1)
      {
          int head1 = head.load(std::memory_order_acquire);
		  printf("thread 1: reads head = %d in Pop()\n", head1);
		  usleep(1500);
          int next1 = array[head1].Next.exchange(-1, std::memory_order_seq_cst);
		  printf("thread 1: executes Exchange on the Next field of the element %d, he reads %d and writes -1\n", head1, next1);
		  
			
          if (next1 >= 0)
          {
              int head2 = head1;
			  usleep(2000);
			  printf("head2 = %d, head1 = %d\n", head2, head1);
              if (head.compare_exchange_strong(head2, next1, std::memory_order_seq_cst))
              {
                  printf("thread 1: succeeds with CompareExchange in Pop(), now head = %d (however head must be equal to 2!)\n", head.load(std::memory_order_acquire));
				  usleep(50000);
				  printf("@@@ thread 1: wakeup\n", head.load(std::memory_order_acquire));
				  count.fetch_sub(1, std::memory_order_seq_cst);
                  return head1;
              }
              else
              {
                  array[head1].Next.exchange(next1, std::memory_order_seq_cst);
              }
          }
      }

      return -1;
  }
  
  int Pop2()
  {
	  static int f2 = 1;
	  while (count.load(std::memory_order_acquire) > 1)
      {
          int head1 = head.load(std::memory_order_acquire);
		  if (f2 == 1)
			f2 = 0;
		  else {
			printf("thread 2: reads head = %d in Pop()\n", head1);
			usleep(15000);
		  }
          int next1 = array[head1].Next.exchange(-1, std::memory_order_seq_cst);
		  printf("thread 2: reads next1 = %d in Pop()\n", next1);

          if (next1 >= 0)
          {
              int head2 = head1;
              if (head.compare_exchange_strong(head2, next1, std::memory_order_seq_cst))
              {
                  count.fetch_sub(1, std::memory_order_seq_cst);
                  return head1;
              }
              else
              {
                  array[head1].Next.exchange(next1, std::memory_order_seq_cst);
              }
          }
      }

      return -1;
  }

  void Push0(int index)
  {
      printf("thread 0: starts pushing the element %d\n", index);
	  int head1 = head.load(std::memory_order_acquire);
	  static int flag = 0;
	  int dof = 0;
      do
      {
		  array[index].Next.store(head1, std::memory_order_release);
		  if (dof == 1)
			printf("thread 0: fails with CompareExchange in Push(), re-reads head=%d, and writes %d to the Next field of the element %d.\n", head.load(std::memory_order_acquire), head1, index);
		  
		  if (dof == 0)
			printf("thread 0: reads head = %d, and sets the Next field of the element %d to %d\n", head.load(std::memory_order_acquire), index,head1);
		
		  if (dof == 0){
			usleep(2000);
			dof = 1;
		  }
         
      } while (!head.compare_exchange_strong(head1, index, std::memory_order_seq_cst));
	  printf("thread 0: succeeds with CompareExchange in Push(), now head=%d\n", head.load(std::memory_order_acquire), index);
		  
      count.fetch_add(1, std::memory_order_seq_cst);
	  printf("thread 0: now count = %d\n", count.load(std::memory_order_acquire));
	  if (flag == 1){
		usleep(2000);
	  }
	  flag = 1;
  }
  
  void Push1(int index)
  {
      int head1 = head.load(std::memory_order_acquire);      
	  do
      {
		  array[index].Next.store(head1, std::memory_order_release);
         
      } while (!head.compare_exchange_strong(head1, index, std::memory_order_seq_cst));
      count.fetch_add(1, std::memory_order_seq_cst);
  }
  
  void Push2(int index)
  {
      int head1 = head.load(std::memory_order_acquire);
      int flag = 0;
	  do
      {
          array[index].Next.store(head1, std::memory_order_release);
		  printf("thread 2: reads head = %d, and sets the Next field of the element %d to %d\n", head.load(std::memory_order_acquire), index, head1);
         
      } while (!head.compare_exchange_strong(head1, index, std::memory_order_seq_cst));
      count.fetch_add(1, std::memory_order_seq_cst);
  }
};

const unsigned NUM_THREADS = 3;
SafeStack<int> stack(3);

pthread_t threads[NUM_THREADS];

void* thread0(void* arg)
{
	usleep(500);
	int idx = (int)(size_t)arg;
    for (size_t i = 0; i != 2; i += 1)
    {	
        if(i==1) {
			printf("T0 sleeping\n");
			usleep(2000);
			printf("T0 get up\n");
			printf("T0 now count = %d\n", stack.getCount());
		}
		int elem;
        for (;;)
        {
			elem = stack.Pop0();
            if (elem >= 0){
                printf("thread 0: get element %d from stack, now head = %d;\n", elem, stack.getHead());
				break;
			}
        }

        stack.array[elem].Value = idx;
		if(i==1){
			printf("--> thread 0: work on elem = %d, idx = %d\n", elem, idx);
			usleep(30000);
		}
        assert(stack.array[elem].Value == idx);
		if(i==1){
			printf("--> thread 0: finished assertion checking\n", elem, idx);
		}
        stack.Push0(elem);
    }
    return NULL;
}

void* thread1(void* arg)
{
	int idx = (int)(size_t)arg;
	for (size_t i = 0; i != 2; i += 1)
    {
        int elem;
        for (;;)
        {
            elem = stack.Pop1();
            if (elem >= 0){
				printf("thread 1: get element %d from stack\n", elem);
                break;
			}
        }
        stack.array[elem].Value = idx;
		printf("--> thread 1: work on elem = %d, idx = %d\n", elem, idx);
        assert(stack.array[elem].Value == idx);

        stack.Push1(elem);
		
    }
    return NULL;
}

void* thread2(void* arg)
{
	usleep(2000);
	int idx = (int)(size_t)arg;
	for (size_t i = 0; i != 2; i += 1)
    {
        int elem;
        for (;;)
        {
            elem = stack.Pop2();
            if (elem >= 0){
				printf("thread 2: get element %d from stack, now head = %d;\n", elem, stack.getHead());
                break;
			}
        }
		
        stack.array[elem].Value = idx;
		if(i==1){
			printf("--> thread 2: work on elem = %d, idx = %d\n", elem, idx);
			usleep(150000);
		}
        assert(stack.array[elem].Value == idx);
		if(i==1){
			printf("--> thread 0: finished assertion checking\n", elem, idx);
		}
        stack.Push2(elem);
    }
    return NULL;
}

int main()
{
	int i = 0;
    pthread_create(&threads[0], NULL, thread0, (void*)i);
	pthread_create(&threads[1], NULL, thread1, (void*)(i+1));
	pthread_create(&threads[2], NULL, thread2, (void*)(i+2));

    pthread_join(threads[2], NULL);
	pthread_join(threads[1], NULL);
	pthread_join(threads[0], NULL);
  
	return 0;
}

