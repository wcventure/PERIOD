# A Prototype for Deterministic Scheduling using SCHED_DEADLINE

The uncertainty in scheduling has always posed a huge challenge for testing
and debugging multithreaded programs.
Some bugs that can only be triggered by a particular thread interleaving may
never be spotted in a testing environment using the default scheduling methods.
Therefore, we propose a deterministic scheduling method that can explore all
the interesting thread interleaving, based on the periodic
[deadline scheduling][dl_doc] method provided in the Linux kernel,
which we call the Deadline Based Deterministic Scheduling (DBDS).

## Quick Start

To test the Deadline Based Deterministic Scheduling (DBDS):
```bash
$ gcc -pthread increase_double_dl_comp.c -o increase_double_dl_comp
$ ./run_increase_double_dl_comp.py
```
Note that `sudo` privileges needed.
For more information, see [here](#deadline-based-deterministic-scheduling-dbds).

To test the DBDS-Lite:
```bash
$ gcc -pthread increase_double_dl.c -o increase_double_dl
$ ./run_increase_double_dl.py
```
Note that `sudo` privileges needed.
For more information, see [here](#dbds-lite).

DBDS-Lite has better performance in a single cpu environment.
To bind your current shell and all the future commands running in it
to cpu 0, use the following command:
```bash
$ sudo ./bind_pid_cpu.sh $$ 0
```
Note that, `sudo` privileges needed; the last argument `0` can be
replaced with any valid cpu ID.

### Interleaving Iterator

In `interleaving.py`, `interleaving_iter` is a 'generator' function.
It returns an iterator that will give an interleaving each time it
is visited.
For instance, in the following example, we print interleaving for 3
threads with 2 key points each:
```python
>>> from interleaving import *
>>> iter = interleaving_iter(3, [2])
>>> for i in iter:
...   print(i)
...
[0, 0, 1, 1, 2, 2]
[0, 0, 1, 2, 1, 2]
...
```
By using `interleaving_iter`, you can also 'extend' interleaving with
more key points.
Supposing that after running the second interleaving `[0, 0, 1, 2, 1, 2]`,
you find 3 more key points are covered (one for each thread).
This is very likely caused by using the new interleaving with the prefix
`[0, 0, 1, 2, ...]` that is different from the previous one with prefix
`[0, 0, 1, 1, ...]`.
Therefore, you'd like to keep the `[0, 0, 1, 2, ...]` prefix and extend
the interleaving with more key points.
To do this, you should simply provide the 'interesting' interleaving to
the function as a 'parent' interleaving, and change the number of KPs:
```python
>>> from interleaving import *
>>> iter = interleaving_iter(3, [3], [0, 0, 1, 2, 1, 2])
>>> for i in iter:
...   print(i)
...
[0, 0, 1, 2, 0, 1, 1, 2, 2]
[0, 0, 1, 2, 0, 1, 2, 1, 2]
...
```
As you will see, the new interleaving patterns generated have the same
prefix as their parent.

## Methodologies

### Introducing Key Points

One of our insight is that if threads do not access shared resources,
they can be considered as running in isolation and different interleaving
will have no impact on the execution results.
Only when there are some shared resources/variables, the different orders
in which the threads access them may have visible effects.

Therefore, to explore as many program behaviours as possible, we can focus
on only the different interleaving of key-point-accessing instructions.

### Calculating Possible Interleaving

With the notion of key points introduced, we can calculate all possible
interleaving of the key-point-accessing instructions, using the `interleaving_gen`
function provided in `interleaving.py`.

There are two mandatory parameters: the first parameter `N` indicates how many
threads in the program we are looking at; and the second
parameter `M` is the number of key points in each thread.
Different threads may have different numbers of key points in reality;
but for simplicity, we use the largest number of key points for every thread.
With the specified information, `interleaving_gen` calculates all possible
interleaving about the key points.

For instance, we can calculate all the interleaving for two threads that
each has three key points:

```python
>>> import interleaving
>>> interleaving.interleaving_gen(N=2,M=3,to_print_num=True,to_print_rlt=True)
Number of interleaving:  20
[0, 0, 0, 1, 1, 1]
[0, 0, 1, 0, 1, 1]
[0, 0, 1, 1, 0, 1]
[0, 0, 1, 1, 1, 0]
[0, 1, 0, 0, 1, 1]
[0, 1, 0, 1, 0, 1]
[0, 1, 0, 1, 1, 0]
[0, 1, 1, 0, 0, 1]
[0, 1, 1, 0, 1, 0]
[0, 1, 1, 1, 0, 0]
[1, 0, 0, 0, 1, 1]
[1, 0, 0, 1, 0, 1]
[1, 0, 0, 1, 1, 0]
[1, 0, 1, 0, 0, 1]
[1, 0, 1, 0, 1, 0]
[1, 0, 1, 1, 0, 0]
[1, 1, 0, 0, 0, 1]
[1, 1, 0, 0, 1, 0]
[1, 1, 0, 1, 0, 0]
[1, 1, 1, 0, 0, 0]
```

### Deadline Based Deterministic Scheduling DBDS

In the previous section, we demonstrated how to calculate the interleaving.
If we can test all of them, we will know there is no place for bugs to hide.
However, using the default scheduler, some interleaving may never be covered
within limited testing rounds.
To tackle these problems, we introduce a SCHED_DEADLINE based deterministic
scheduling method.

The current Linux kernel provides several [scheduling policies][sched_doc],
namely, SCHED_OTHER, SCHED_IDLE, SCHED_BATCH, SCHED_FIFO, SCHED_RR, and
SCHED_DEADLINE.

Originally, the deadline scheduling method is designed for periodic
tasks, e.g., rendering a frame for a video player every 10ms.
It takes three arguments to set up a deadline-driven thread, namely
the `runtime`, `deadline`, and `period`.
At the beginning of each period, the scheduler will wake the thread
and allow it to execute at most `runtime` before the `deadline`.
If the thread finishes early, it can call the function `sched_yield()`
to give up running until the next period.

However, we use the deadline scheduling in a novel way to devise a
deterministic scheduling method.

Firstly, we assume all working threads are created at roughly the same time.
Then, we set all of them to be deadline-driven tasks with the same period
length that long enough to contain the most time-consuming thread's execution.
In this way, we have all threads' periods aligned.

For instance, the scheduling for two threads with three key-point-accessing
instructions each would look like the following:

```
                  Thread 0               Thread 1
            +-----------------+    +-----------------+
            | instruction 0.0 |    | instruction 1.0 |   
period 0    | instruction 0.1 |    | instruction 1.1 |
            | instruction 0.2 |    | instruction 1.2 |
            +-----------------+    +-----------------+
            |                 |    |                 |
period 1    |                 |    |                 |
            |                 |    |                 |
            +-----------------+    +-----------------+
...         |       ...       |    |      ...        |
```

However, this is still not enough.
As the periods are sufficiently long, instructions from both threads will all
fall in their first period (period 0), leading to uncertainty in interleaving.
To solve this problem, the `sched_yield()` function comes in handy.
Intuitively, a `sched_yield()` will push the instructions after it to
the next periodic block.
For instance, if we add a `sched_yield()` before `instruction 1.0`, the
whole thread 1 will be scheduled after thread 0:

```
                  Thread 0               Thread 1
            +-----------------+    +-----------------+
            | instruction 0.0 |    |  sched_yield()  |   
period 0    | instruction 0.1 |    |                 |
            | instruction 0.2 |    |                 |
            +-----------------+    +-----------------+
            |                 |    | instruction 1.0 |   
period 1    |                 |    | instruction 1.1 |
            |                 |    | instruction 1.2 |
            +-----------------+    +-----------------+
...         |       ...       |    |      ...        |
```

This scheduling is first of twenty possible scheduling we calculated previously.
We say it's yield pattern is `[0, 0, 0] [1, 0, 0]`.
If we want to get a different scheduling, we can use a different yield pattern.
Let us take the 10th scheduling `[0, 1, 1, 1, 0, 0]` as an example; we can add
two yields before inst. 0.1 and one before inst. 1.0, i.e., using the yield
pattern `[0, 2, 0] [1, 0, 0]`:

```
                  Thread 0               Thread 1
            +-----------------+    +-----------------+
            | instruction 0.0 |    | sched_yield()   |   
period 0    | sched_yield()   |    |                 |
            |                 |    |                 |
            +-----------------+    +-----------------+
            | sched_yield()   |    | instruction 1.0 |   
period 1    |                 |    | instruction 1.1 |
            |                 |    | instruction 1.2 |
            +-----------------+    +-----------------+
            | instruction 0.1 |    |                 |
period 2    | instruction 0.2 |    |                 |
            |                 |    |                 |
            +-----------------+    +-----------------+
...         |       ...       |    |      ...        |
```

In this way, we can enforce any interleaving we are interested in,
and avoid any uncertainty caused by overlapping instructions, by ensuring
no two threads' instructions fall in the same scheduling period.

We provide the `yield_pattern_gen` function in `interleaving.py` to
calculate the yield pattern needed to get the given interleaving
(`inter`) for `N` threads:

```python
>>> import interleaving
>>> inter = [0, 1, 1, 1, 0, 0]
>>> interleaving.yield_pattern_gen(N=2, inter=inter, to_print=True)
[0, 1, 1, 1, 0, 0] [0, 2, 0] [1, 0, 0]
```

#### Demo

We provide a demo for our deadline based deterministic scheduling.

Target program is `increase_double_dl_comp.c`
Test driver: `run_increase_double_dl_comp.py`

To run the demo, compile the target `.c` file first; then, run the
test driver written in python to see the analysis.

```bash
$ gcc -pthread increase_double_dl_comp.c -o increase_double_dl_comp
$ ./run_increase_double_dl_comp.py
Number of interleaving:  20
[0, 0, 0, 1, 1, 1] [0, 0, 0] [1, 0, 0]
[0, 0, 1, 0, 1, 1] [0, 0, 2] [1, 2, 0]
[0, 0, 1, 1, 0, 1] [0, 0, 2] [1, 0, 2]
[0, 0, 1, 1, 1, 0] [0, 0, 2] [1, 0, 0]
[0, 1, 0, 0, 1, 1] [0, 2, 0] [1, 2, 0]
[0, 1, 0, 1, 0, 1] [0, 2, 2] [1, 2, 2]
[0, 1, 0, 1, 1, 0] [0, 2, 2] [1, 2, 0]
[0, 1, 1, 0, 0, 1] [0, 2, 0] [1, 0, 2]
[0, 1, 1, 0, 1, 0] [0, 2, 2] [1, 0, 2]
[0, 1, 1, 1, 0, 0] [0, 2, 0] [1, 0, 0]
[1, 0, 0, 0, 1, 1] [1, 0, 0] [0, 2, 0]
[1, 0, 0, 1, 0, 1] [1, 0, 2] [0, 2, 2]
[1, 0, 0, 1, 1, 0] [1, 0, 2] [0, 2, 0]
[1, 0, 1, 0, 0, 1] [1, 2, 0] [0, 2, 2]
[1, 0, 1, 0, 1, 0] [1, 2, 2] [0, 2, 2]
[1, 0, 1, 1, 0, 0] [1, 2, 0] [0, 2, 0]
[1, 1, 0, 0, 0, 1] [1, 0, 0] [0, 0, 2]
[1, 1, 0, 0, 1, 0] [1, 0, 2] [0, 0, 2]
[1, 1, 0, 1, 0, 0] [1, 2, 0] [0, 0, 2]
[1, 1, 1, 0, 0, 0] [1, 0, 0] [0, 0, 0]
10 results found: ['16', '12', '10', '9', '8', '6', '5', '4', '3', '2']
--------------------------------------------------
        Last New Find       Total
Round   20                  20
Time    00:00:01.03551      00:00:01.03569
```

Specifically speaking, each time of running the target program takes a
series of arguments to guide the threads to form a certain interleaving.
The parameters include the number of threads `N`, the key points in each
thread `M`, and `N * M` integers indicating how many times a thread should
yield before the corresponding key point.
For instance, the following command will ask the target program to run
following the 10th scheduling:

```bash
$ sudo ./increase_double_dl_comp 2 3 0 2 0 1 0 0
t_1: started
t_1: 1 times to yield at KP 1.0
t_0: started
t_0: 0 times to yield at KP 0.0
t_0: x = 1
t_0: 2 times to yield at KP 0.1
t_1: 0 times to yield at KP 1.0
t_1: x = 2
t_1: 0 times to yield at KP 1.1
t_0: 1 times to yield at KP 0.1
t_1: x = 4
t_1: 0 times to yield at KP 1.2
t_1: x = 8
t_1: finished
t_0: 0 times to yield at KP 0.1
t_0: x = 9
t_0: finished
x = 9
```

### DBDS-Lite

Our DBDS thoroughly explores all possible interleaving for the
key-point-accessing instructions.
However, sometime an incomplete yet fast exploration could be
better suited for testing purposes.
Therefore, we also offer a lite version of DBDS that uses a simplified
encoding to guide the threads to yield and to form different interleaving.

This time, the program takes as arguments the number of threads `N`,
and another `N` integers as instructors (one for each thread).
A thread will yield once or not at all before one of its key points,
depending on the corresponding bit in the binary representation of
its instructor number.
For instance, if a thread's instructor number is `2`, whose binary
representation is `10`, it will yield before its second key point,
but not before its first one.

We've provided a demo for DBDS-Lite:

```bash
$ gcc -pthread increase_double_dl.c -o increase_double_dl
$ ./run_increase_double_dl.py
10 results found: ['16', '8', '2', '4', '10', '6', '9', '12', '5', '3']
--------------------------------------------------
        Last New Find       Total
Round   37                  100
Time    00:00:00.51299      00:00:01.49947
```

### Running Test using a Single CPU

For DBDS-Lite, the instructor numbers are easy to generate; however,
it is difficult to guarantee that the instructor number compositions
will make the execution overlapping free, which means DBDS-Lite may
still subject to uncertainty.

According to our experiments, the DBDS-Lite method generally performs
better, i.e., tends to cover more different scheduling, when running
on a single CPU system.
Therefore, we introduce a bash script to help user to set up a single
CPU environment.

Execute the following command to bind your current shell and all the
future commands running in it to CPU 0:
```bash
$ sudo ./bind_pid_cpu.sh $$ 0
```
Note that, the last argument `0` can be replaced with any valid CPU ID.


[dl_doc]: https://www.kernel.org/doc/html/latest/scheduler/sched-deadline.html
[sched_doc]: https://man7.org/linux/man-pages/man7/sched.7.html


### Tip

This tip is for systems with multi-core processors. To check information about your CPU, run this command from the Terminal:
```bash
cat /proc/cpuinfo
```
To disable a CPU core, run now this command:
```bash
echo 0 | sudo tee /sys/devices/system/cpu/cpu1/online
```
To enable it, use this command:
```bash
echo 1 | sudo tee /sys/devices/system/cpu/cpu1/online
```
