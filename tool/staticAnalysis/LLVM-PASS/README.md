# Instrumentation

This page shows how to perform our instrumentation to the target program. The instrumented program will be used in [Deadline Based Deterministic Scheduling (DBDS)](https://github.com/wcventure/ConcurrencyFuzzer/tree/master/tool/DBDS) process.

-----

## Quick Start

First, compile the target program into LLVM bit code. After that perform the script that integrates the static analysis and Instrumentation.  

```bash
# get bit code file
$ export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
$ export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
$ cd $ROOT_DIR/test/increase_double
$ clang -g -emit-llvm -c ./increase_double.c -o increase_double.bc

# Run static analysis and Instrumentation
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh increase_double
```

Then you will see the instrumented program namely `dbds.increase_double` under the working folder. And you can run a python script to start the DBDS by passing on the instrumented program as the parameter.

-----

## Detail Workflow

This Section is too detailed. For ordinary users, using the top-level script we provided is enough. Here we only describe the workflow of our script, as well as the main component. We omit the implementation detail.

The instrumentation consists of two parts. One is [Dry Run Instrumentation](#dry-run), another is [DBDS Instrumentation](#instrumentation).


### Dry Run Instrumentation

Before exploring interleaving, we need to collect some dynamic information (e.g. actual running thread, the number of key points execute, and the execution time). We call this process as a dry run.

Take the following program ([`increase_double.c`](test/increase_double/increase_double.c)) as an example.

```C
#include <pthread.h>
#include <stdio.h>

int x = 0;

// this function increases x by 1 in each iteration
void *incr(void *data) {
    for(int i = 0; i < 2; i++) {
        x += 1;     // key point 0
    }
    printf("finished 1\n");
}

// this function doubles x in each iteration
void *doub(void *data) {
    for(int i = 0; i < 3; i++) {
        x *= 2;     // key point 1
    }
    printf("finished 2\n");
}

int main(int argc, char *argv[]) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, incr, NULL);
    pthread_create(&t2, NULL, doub, NULL);
    pthread_join(t2, NULL);
    pthread_join(t1, NULL);
	
    printf("x = %d\n", x);
    return 0;
}
```

This program will create two child threads. One thread will execute the function `incr()`, another will execute the function `doub()`. The function `incr()` will increase the global variable x by 1 in two iterations. And the function `doub()` will double the global variable x in three iterations.

After the static analysis, we can find that there are 3 key points that may leads to data race.

```sh
x += 1;                     // key point 0
x *= 2;                     // key point 1
printf("x = %d\n", x);      // key point 1
```

However, it is still difficult for static analysis to tell us how many times each key point will execute, as the number of loop iterations is uncertain. So we next perform a dry run to collects dynamic information. 

You can use the following command to obtain the instrumented program for the dry run.

```bash
$ export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
$ export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
$ cd $ROOT_DIR/test/increase_double
$ clang -g -emit-llvm -c ./increase_double.c -o increase_double.bc
$ export PATH=$ROOT_DIR/tool/SVF/Release-build/bin/:$PATH
$ $ROOT_DIR/tool/staticAnalysis/analysis.py ./increase_double.bc > ./mssa.increase_double
$ $ROOT_DIR/tool/staticAnalysis/get_aliased_pair.py ./mssa.increase_double > ./mempair_all.increase_double
$ $ROOT_DIR/tool/staticAnalysis/prune.py ./mempair_all.increase_double > ./mempair.increase_double
$ $ROOT_DIR/tool/staticAnalysis/ExtractLineStdout.py -f ./mempair.increase_double > ./ConConfig.increase_double
$ export Con_PATH=${ROOT_DIR}/test/increase_double/ConConfig.increase_double
$ opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DryRunPass/libDryRunPass.so -Instru ./increase_double.bc -o dryRun.increase_double.bc
$ clang++ -g ./dryRun.increase_double.bc $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DryRunPass/libinstruFunction.a -o dryRun.increase_double -lpthread -ldl -rdynamic
```

We can see the log (`func_trace.log`) as follows. This log can tell us how many threads actually have been created, how many times each key point actually executes, and the actual execution time under instrumentation.

```sh
Thread 15818 (140598670788416) enter function: main
Tid: 15818, Function: main, KeyPoint: 0.
Thread 0 (140598648571648) execute pthread_create(incr)
Thread 15819 (140598648571648) enter function: incr
Tid: 15819, Function: incr, KeyPoint: increase_double.c:9.
Tid: 15819, Function: incr, KeyPoint: increase_double.c:9.
Tid: 15819, Function: incr, KeyPoint: increase_double.c:9.
Tid: 15819, Function: incr, KeyPoint: increase_double.c:9.
Thread 0 (140598640117504) execute pthread_create(doub)
Tid: 15819, Function: incr, KeyPoint: increase_double.c:9.
Thread 15820 (140598640117504) enter function: doub
Tid: 15819, Function: incr, KeyPoint: increase_double.c:9.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Tid: 15820, Function: doub, KeyPoint: increase_double.c:17.
Thread 15819 (140598648571648) exit function: incr
Thread 15819 (140598648571648) execute pthread_join()
Thread 15820 (140598640117504) exit function: doub
Tid: 15818, Function: main, KeyPoint: increase_double.c:30.
Thread 15820 (140598640117504) execute pthread_join()
Tid: 15818, Function: main, KeyPoint: increase_double.c:30.
Tid: 15818, Function: main, KeyPoint: increase_double.c:30.
Thread 15818 (140598670788416) exit function: main
Summary(Time: 0.004192 (s))
Summary(15818: 4, 15819: 6, 15820: 9)
Summary(ThraedNum:3, KPNum:9)
```


### DBDS Instrumentation

The DBDS instrumentation will instrument the Deterministic Scheduling adjustment code. For example, the instrumented program for ([`increase_double.c`](test/increase_double/increase_double.c)) can bee seen as follows. (The instrumentation for the following program is done manually. In our implementation of generalizing this process, it is slightly different.)

```C
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include "sched.h"

const int N1 = 2;
const int N2 = 3;

int x = 0;

// F: code blocks annotated with "// F:" are introduced to guide fuzzing
int num_td;     // number of threads concerned
int num_kp;     // number of kep points per thread
int kp[10000];  // how many times should a thread yield before a kp

// this function increases x by 1 in each iteration
void *incr(void *data) {
    while(get_tid_to_run() == 0);		// F: wait until its his turn
    int sched_tid = get_tid_to_run();	// F: acquire the tid for sechduling
    set_tid_to_run(0);					// F: notify main thread to run

    // F: set the thread to use deadline scheduling
    set_sched_dl(1000 * 1000, 1000 * 1000 * 10, 1000 * 1000 * 10);
    int idx_kp = (sched_tid - 1) * num_kp;

    printf("t_%d: started\n", sched_tid);

    for(int i = 0; i < N1; i++) {

        // F: give up several circles' time
        while(1) {
            printf("t_%d: %d times to yield at KP %d.%d\n", sched_tid,
			       kp[idx_kp], sched_tid, idx_kp - (sched_tid - 1) * num_kp);
            if(kp[idx_kp] <= 0) break;
            sched_yield();
            kp[idx_kp]--;
        }
        idx_kp++;

        x += 1;     // a key point
		printf("t_%d: x = %d\n", sched_tid, x);
    }
    printf("t_%d: finished\n", sched_tid);
}

// this function doubles x in each iteration
void *doub(void *data) {
    while(get_tid_to_run() == 0);       // F: wait until its his turn
    int sched_tid = get_tid_to_run();   // F: acquire the tid for sechduling
    set_tid_to_run(0);                  // F: notify main thread to run

    // F: set the thread to use deadline scheduling
    set_sched_dl(1000 * 1000, 1000 * 1000 * 10, 1000 * 1000 * 10);
    int idx_kp = (sched_tid - 1) * num_kp;

    printf("t_%d: started\n", sched_tid);

    for(int i = 0; i < N2; i++) {

        // F: give up several circles' time
        while(1) {
            printf("t_%d: %d times to yield at KP %d.%d\n", sched_tid,
                    kp[idx_kp], sched_tid, idx_kp - (sched_tid - 1) * num_kp);
            if(kp[idx_kp] <= 0) break;
            sched_yield();
            kp[idx_kp]--;
        }
        idx_kp++;

        x *= 2;         // a key point
        printf("t_%d: x = %d\n", sched_tid, x);
    }
    printf("t_%d: finished\n", sched_tid);
}

int main(int argc, char *argv[]) {
    // F: set up the yield patten
    num_td = (int) strtol(argv[1], NULL, 10);
    num_kp = (int) strtol(argv[2], NULL, 10);
    for(int i = 0; i < num_td * num_kp; i++)
        kp[i] = (int) strtol(argv[i + 3], NULL, 10);

    // for(int i = 0; i < 20; i++)
    //     printf("%d ", kp[i]);
    // printf("\n");

    set_tid_to_run(0);      // F: allow the main thread to run
    int c_tid = 0;          // F: child tid, initially 0


    pthread_t t1, t2, t_dummy;

    while(get_tid_to_run() != 0);       // F: wait until its his turn
    c_tid++;                            // F: get the next child tid
    set_tid_to_run(c_tid);              // F: notify thread 1 to get ready

    pthread_create(&t1, NULL, incr, NULL);

    while(get_tid_to_run() != 0);       // F: wait until its his turn
    c_tid++;                            // F: get the next child tid
    set_tid_to_run(c_tid);              // F: notify thread 2 to get ready
	
    pthread_create(&t2, NULL, doub, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // for(int i = 0; i < 20; i++)
    //     printf("%d ", kp[i]);
    // printf("\n");

    printf("x = %d\n", x);

    return 0;
}

```

You can use the following command to obtain the instrumented program for DBDS.

```bash
$ export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
$ export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
$ cd $ROOT_DIR/test/increase_double
$ clang -g -emit-llvm -c ./increase_double.c -o increase_double.bc
$ export PATH=$ROOT_DIR/tool/SVF/Release-build/bin/:$PATH
$ $ROOT_DIR/tool/staticAnalysis/analysis.py ./increase_double.bc > ./mssa.increase_double
$ $ROOT_DIR/tool/staticAnalysis/get_aliased_pair.py ./mssa.increase_double > ./mempair_all.increase_double
$ $ROOT_DIR/tool/staticAnalysis/prune.py ./mempair_all.increase_double > ./mempair.increase_double
$ $ROOT_DIR/tool/staticAnalysis/ExtractLineStdout.py -f ./mempair.increase_double > ./ConConfig.increase_double
$ export Con_PATH=${ROOT_DIR}/test/increase_double/ConConfig.increase_double
$ opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DBDSPass/libDBDSPass.so -Instru ./increase_double.bc -o dbds.increase_double.bc
$ clang++ -g ./dbds.increase_double.bc $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DBDSPass/libDBDSFunction.a -o dbds.increase_double -lpthread -ldl -rdynamic
```

Then to get different result under certain interleavings. 

```sh
$ sudo ./increase_double 2 3 0 2 0 1 0 0
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