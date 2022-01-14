# Exploration Strategy

The bugs we are searching for are probabilistic ones in testing concurrent programs. The more interleavings we explore, the more chances to find the bugs. In our approach, the concurrency is emulated by interleaving the program fragments of different threads. But there are still many possibilities of thread interleaving of program fragments. The search space is over schedules. The number of interleavings depends on the number of scheduling points.

To give ourselves the maximum chance of running across the right interleaving to trigger the bug, we should plan to develop strategies to explore the possible interleaving. We propose four search strategies, in order to strike a balance between completeness and efficiency：
- [global exploration](#global-exploration)
- [random exploration](#random-exploration)
- [directed Exploration](#directed-exploration)
- [incremental Exploration](#incremental-exploration)


### Global exploration

This strategy is usually used when the number of scheduling points we are interested in is small, or the scale of the program is small. During dynamic testing, we can run out all possible interleaving for testing, in an acceptable range.

For instance, there is a program with two threads ($T_0$ and $T_1$) that each has three interesting scheduling points. We can start with running the schedule $<T_0, T_0, T_0, T_1, T_1, T_1>$, and traverse all 20 possible interleavings:
```sh
<T0, T0, T0, T1, T1, T1>
<T0, T0, T1, T0, T1, T1>
<T0, T0, T1, T1, T0, T1>
<T0, T0, T1, T1, T1, T0>
<T0, T1, T0, T0, T1, T1>
<T0, T1, T0, T1, T0, T1>
<T0, T1, T0, T1, T1, T0>
<T0, T1, T1, T0, T0, T1>
<T0, T1, T1, T0, T1, T0>
<T0, T1, T1, T1, T0, T0>
<T1, T0, T0, T0, T1, T1>
<T1, T0, T0, T1, T0, T1>
<T1, T0, T0, T1, T1, T0>
<T1, T0, T1, T0, T0, T1>
<T1, T0, T1, T0, T1, T0>
<T1, T0, T1, T1, T0, T0>
<T1, T1, T0, T0, T0, T1>
<T1, T1, T0, T0, T1, T0>
<T1, T1, T0, T1, T0, T0>
<T1, T1, T1, T0, T0, T0>
```

By employ `SCHED_YIELD` operations before scheduling points, It is possible to run across all 20 interleavings.

As we've explored all the interleaving we're interested in (based on the program fragment divided by scheduling points), leaving those uninteresting interleaving out of consideration, we can guarantee the existence of bugs to a certain extent.


### Random exploration

Most of the time, we may not have enough resources to go through all the possible interleavings. We need to use limited resources to explore more possible interleavings.

we can randomly introduce `SCHED_YIELD` operations before scheduling points to explore more possible interleavings. For instance, there is a program with two threads that each has $n$ interesting scheduling points. The generated yield pattern would look like this:

[$a_1=random()$, $a_2=random()$, $a_3=random()$, $...$, $a_n=random()$] [$b_1=random()$, $b_2=random()$, $b_3=random()$, $...$, $b_n=random()$]

We randomly generate yield pattern based on the following heuristic rules:

- $$a_1 + a_2 + a_3 + ... + a_n < n$$
- $$b_1 + b_2 + b_3 + ... + b_n < n$$
- $$\forall i < n, \forall j < n \Longrightarrow a_1 + a_2 + ... + a_i \neq b_1 + b_2 + ... + b_j$$

Based on the randomly generated yield pattern, the `SCHED_YIELD` operations will be employed before scheduling points during the program execution. Because the yield pattern is probabilistic. To explore more possible interleavings, we should plan to perform testing for much longer.


### Directed Exploration

Global exploration focuses on the completeness, but it is resource-consuming. Random exploration focuses on running across to a certain interleaving in probability but lack of efficiency is not high.

We propose a directed exploration strategy, which focuses on only one subset of scheduling points at a time. A simple intuition is that the trigger of a bug is often only related to a subset of scheduling points rather than all the scheduling points. For instance, there are two independent variables $Var_a$ and $Var_b$. The read/write on $Var_a$ and $Var_b$ do not affect each other's values. If we want to witness the different values of $Var_a$, we only need to consider the scheduling points related to $Var_a$, leaving those scheduling points related to $Var_b$ out of consideration.

Although sometimes there are control dependences and data dependences between two variables, we can also consider them separately during exploration, which helps to reduce the search space. Take the example above, as shown in the following:

```C
// only consider the lock
                                    Thread 0                                                                Thread 1
            +-----------------------------------------------------------+     +-----------------------------------------------------------+
            | S00: // ...                                               |     | S10: // ...                                               |
            | S01: char a = 'a';                                        |     | S11: char c = 'c';                                        |
            | S02: pthread_mutex_lock(i_mutex); //Scheduling Point 1    |     | S12: pthread_mutex_lock(i_mutex); //Scheduling Point 2    |
            | S03: ptr[0] = a;                                          |     | S13: ptr[3] = c;                                          |
period 0    | S04: ptr[1] = a+1;                                        |     | S14: ptr[4] = c+1;                                        |
            | S05: pthread_mutex_unlock(i_mutex);                       |     | S15: ptr[5] = c+2;                                        |
            |                                                           |     | S16: pthread_mutex_unlock(i_mutex);                       |
            |                                                           |     | S17: free(ptr);                                           |
            +-----------------------------------------------------------+     +-----------------------------------------------------------+
            |                                                           |     |                                                           |
```

If we only regard the `pthread_mutex_lock(i_mutex)` statement as the scheduling point, then we can easily interfere with the execution order of program fragments inside the `lock` (This is also very helpful for deadlock detection). There are two cases of interleaving for scheduling only the scheduling point of `pthread_mutex_lock(i_mutex)`, as shown in the following. Note that case 1 trigger use-after-free bug, as the `S17: free(ptr);` would be executed in period 1 and `S03` executed in period 2. 

```C
// scheduling mutex_lock i_mutex: case 1 (trigger use-after-free bug)
                              Thread 0                                        Thread 1
            +-----------------------------------------+     +-----------------------------------------+
            | S00: // ...                             |     | S10: // ...                             |
period 0    | S01: char a = 'a';                      |     | S11: char c = 'c';                      |
            |                                         |     |                                         |
            +-----------------------------------------+     +-----------------------------------------+
            |                                         |     | S12: pthread_mutex_lock(i_mutex);       |
            |                                         |     | S13: ptr[3] = c;                        |
period 1    |                                         |     | S14: ptr[4] = c+1;                      |
            |                                         |     | S15: ptr[5] = c+2;                      |
            |                                         |     | S16: pthread_mutex_unlock(i_mutex);     |
            |                                         |     | S17: free(ptr);                         |
            +-----------------------------------------+     +-----------------------------------------+
            | S02: pthread_mutex_lock(i_mutex);       |     |                                         |
            | S03: ptr[0] = a;                        |     |                                         |
period 2    | S04: ptr[1] = a+1;                      |     |                                         |
            | S05: pthread_mutex_unlock(i_mutex);     |     |                                         |
            +-----------------------------------------+     +-----------------------------------------+
            |                                         |     |                                         |

// scheduling mutex_lock i_mutex: case 2
                              Thread 0                                        Thread 1
            +-----------------------------------------+     +-----------------------------------------+
            | S00: // ...                             |     | S10: // ...                             |
period 0    | S01: char a = 'a';                      |     | S11: char c = 'c';                      |
            |                                         |     |                                         |
            +-----------------------------------------+     +-----------------------------------------+
            | S02: pthread_mutex_lock(i_mutex);       |     |                                         |
            | S03: ptr[0] = a;                        |     |                                         |
period 1    | S04: ptr[1] = a+1;                      |     |                                         |
            | S05: pthread_mutex_unlock(i_mutex);     |     |                                         |
            +-----------------------------------------+     +-----------------------------------------+
            |                                         |     | S12: pthread_mutex_lock(i_mutex);       |
            |                                         |     | S13: ptr[3] = c;                        |
period 2    |                                         |     | S14: ptr[4] = c+1;                      |
            |                                         |     | S15: ptr[5] = c+2;                      |
            |                                         |     | S16: pthread_mutex_unlock(i_mutex);     |
            |                                         |     | S17: free(ptr);                         |
            +-----------------------------------------+     +-----------------------------------------+
            |                                         |     |                                         |
```

If we only regard those statements that read/write `ptr[1]` as the scheduling points, as shown as follows, then we can also interfere with the execution order of those statements that access `ptr[1]`. (In fact, if we didn't consider the lock as scheduling point and the `S02` execute first, we would fail to put `S17: free(ptr);` in front of `S04: ptr[1] = a+1;`. Thus our approach just tries to control the interleaving as much as we can do).

```C
// only consider ptr[1]
                                    Thread 0                                                                Thread 1
            +-----------------------------------------------------------+     +-----------------------------------------------------------+
            | S00: // ...                                               |     | S10: // ...                                               |
            | S01: char a = 'a';                                        |     | S11: char c = 'c';                                        |
            | S02: pthread_mutex_lock(i_mutex);                         |     | S12: pthread_mutex_lock(i_mutex);                         |
            | S03: ptr[0] = a;                                          |     | S13: ptr[3] = c;                                          |
period 0    | S04: ptr[1] = a+1;                //Scheduling Point 1    |     | S14: ptr[4] = c+1;                                        |
            | S05: pthread_mutex_unlock(i_mutex);                       |     | S15: ptr[5] = c+2;                                        |
            |                                                           |     | S16: pthread_mutex_unlock(i_mutex);                       |
            |                                                           |     | S17: free(ptr);                   //Scheduling Point 2    |
            +-----------------------------------------------------------+     +-----------------------------------------------------------+
            |                                                           |     |                                                           |
```

The directed exploration tries to pick a subset from all the scheduling points, and apply global exploration on those scheduling points. This strategy allows us to focus on a group of program statements or a pair of the program statement. This strategy strikes a balance between completeness and efficiency, and often makes it easy to find the bug.


### Incremental Exploration

Some bugs are hidden in deep program paths that depend on specific threads interleaving. In order to run across those threads interleaving more efficiently, we propose an incremental exploration strategy.

Take the following program as an example, the `free(ptr);` statement is hidden in a deeply nested branch. And the `free(ptr);` statement can only be executed on a specific thread interleaving.

```C
#include <stdlib.h>
#include <pthread.h>

char* ptr;
int a, b;
pthread_mutex_t i_mutex;

void *t1(void *)
{
      a = 1;
      b = 1;
      ptr[0] = a;
      return NULL;
}

void *t2(void *)
{
      a = 0;
      b = 0;
      if(a){
            if(b){
                  free(ptr);
            }
      }
      return NULL;
}

int main(int argc, char **argv) {

      ptr = (char*)malloc(5);

      pthread_t id1, id2;
      pthread_create(&id1, NULL, t1, NULL);
      pthread_create(&id2, NULL, t2, NULL);
      pthread_join(id2, NULL);
      pthread_join(id1, NULL);

      return 0;
}
```

Assume that we start the testing with the schedule $<T_0, T_0, T_0, T_1, T_1, T_1>$, the execution would look like:

```sh
                             Thread 0                                        Thread 1
            +-----------------------------------+     +-----------------------------------+
            | S00: a = 1;                       |     |                                   |
            | S01: b = 1;                       |     |                                   |
period 0    | ptr[0] = a;                       |     |                                   |
            | return NULL;                      |     |                                   |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     | S10: a = 0;                       |
period 1    |                                   |     | S11: b = 0;                       |
            |                                   |     | S12: if(a)                        |
            |                                   |     | S13: return NULL;                 |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     |                                   |
```

In this situation, the program with two threads ($T_0$ and $T_1$) can only witnesses three scheduling points for each thread. The global exploration with three scheduling points is not enough to cover all the behavior of this concurrent program, as we didn't consider another two uncover scheduling points. 

Using incremental exploration strategy, we still start the testing with the schedule $<T_0, T_0, T_0, T_1, T_1, T_1>$. Once the execution witnesses a new uncovered scheduling point, we would consider the schedule under test as an interesting schedule. And there is a prefix in the interesting schedule that can trigger the new scheduling point. This allows us to continue to perform the exploration inside the deeply nested branch.

For instance, the execution of schedule $<T_1, T_0, T_0, T_0, T_1, T_1>$, which wound execute the statement `if(b)  `, would look like:

```sh
                             Thread 0                                        Thread 1
            +-----------------------------------+     +-----------------------------------+
            |                                   |     | S10: a = 0;                       |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
            | S00: a = 1;                       |     |                                   |
            | S01: b = 1;                       |     |                                   |
period 0    | ptr[0] = a;                       |     |                                   |
            | return NULL;                      |     |                                   |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
period 1    |                                   |     | S11: b = 0;                       |
            |                                   |     | S12: if(a)                        |
            |                                   |     | S13:   if(b)                      |
            |                                   |     | S14: return NULL;                 |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     |                                   |
```

There exeist a prefix of the schedule $<T_1, T_0, T_0, T_0, T_1, T_1, T_1>$ that guarantee the execution to `if(b)` statement. It easy the find the prefix $<T_1, T_0, ...>$. The incremental exploration strategy would try to perform the exploration by extending the prefix $<T_1, T_0, ...>$.

It is easy to find the following interleaving (with schedule $<T_1, T_0, T_1, T_0, T_0, T_1, T_1, T_1>$) that execute to a new uncover statement `free(ptr);`, by extending the prefix $<T_1, T_0, ...>$. The new prefix $<T_1, T_0, T_1, T_0, ...>$ that would guarantee in execute to `free(ptr);` statement would consider as interesting prefix.

```sh
                             Thread 0                                        Thread 1
            +-----------------------------------+     +-----------------------------------+
period 0    |                                   |     | S10: a = 0;                       |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
period 1    | S00: a = 1;                       |     |                                   |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
period 2    |                                   |     | S11: b = 0;                       |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
            | S01: b = 1;                       |     |                                   |
period 3    | ptr[0] = a;                       |     |                                   |
            | return NULL;                      |     |                                   |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     | S12: if(a)                        |
period 4    |                                   |     | S13:   if(b)                      |
            |                                   |     | S14:      free(ptr);              |
            |                                   |     | S15: return NULL;                 |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     |                                   |
```

It is easy to find the following interleaving that trigger a use-after-free bug, by By extending the prefix $<T_1, T_0, T_1, T_0, ...>$.

```sh
                             Thread 0                                        Thread 1
            +-----------------------------------+     +-----------------------------------+
period 0    |                                   |     | S10: a = 0;                       |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
period 1    | S00: a = 1;                       |     |                                   |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
period 2    |                                   |     | S11: b = 0;                       |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
period 3    | S01: b = 1;                       |     |                                   |
            |                                   |     |                                   |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     | S12: if(a)                        |
period 4    |                                   |     | S13:   if(b)                      |
            |                                   |     | S14:      free(ptr);              |
            |                                   |     | S15: return NULL;                 |
            +-----------------------------------+     +-----------------------------------+
period 5    | ptr[0] = a;                       |     |                                   |
            | return NULL;                      |     |                                   |
            +-----------------------------------+     +-----------------------------------+
            |                                   |     |                                   |
```

Thus the incremental exploration strategy is useful for running across the specific interleaving that executes the code inside the nested branch.
