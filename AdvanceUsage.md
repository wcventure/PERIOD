
# Advance Usage (Run PERIOD step by step)

Previously, we recommend using the script `./build` we provided to build the benchmark programs. Here, we introduce how to use PERIOD for general programs. We use an example to quickly show you how to use PERIOD to test a given concurrent program. Consider the following multithreaded program (the code can be found in `/workdir/PERIOD/test/work`).

```C
01 #include <stdio.h>
02 #include <stdlib.h>
03 #include <pthread.h>
04
05 char *g;
06
07 void *Work(void *args) {
08
09      g = (char*)malloc(10); printf("%p\n",g);
10      if (g != NULL) {
11          free(g);
12      }
13      g = NULL;
14 }
15
16 int main(int argc, char **argv) {
17      pthread_t id1, id2;
18      pthread_create(&id2, NULL, Work, NULL);
19      pthread_create(&id1, NULL, Work, NULL);
20      pthread_join(id1, NULL);
21      pthread_join(id2, NULL);
22      printf("finished\n");
23      return 0;
24 }
```

This program creates two threads. Each thread will write the shared variable `g`. In the beginning, both threads try to allocate a section of memory and write the address that points to allocated memory to variable g. Then if `g != NULL`, the memory pointed by g will be released. Finally, set `g = NULL`.

Obviously, this program has a double-free and memory leak bug on the shared variable g. If the `g = (char*)malloc(10)`; in one thread execute following close at the `g = (char*)malloc(10)`; in another thread, a memory leak on g would happen. A double-free can occur if g in both threads become alias, and the `free(g)`; statement in one thread executes following close at the `free(g)`; in another thread, a double-free would happen. We want to see whether PERIOD can quickly find these two bugs and expose the buggy interleaving.

For using PERIOD, you need to perform the following step:
1. Compile the program on LLVM platform and get its bitcode.
2. Perform our static analysis scripts to analyze key points for further scheduling.
3. Import the information of key points to the environment variable  Con_PATH and perform instrumentation on the .bc file.
4. Expose the bug through PERIOD's systematic testing.
5. Reproduce the buggy interleavings if found the bugs.

**Compile the program on LLVM platform and get its bitcode, according to the following rules.**

- Preferably use static linking
- Preferably use -g when compiling
- Preferably use -fno-omit-frame-pointer when compiling, but not required

For example:
```sh
cd $ROOT_DIR/test/work
./cleanDIR.sh
clang -g -O0 -fno-omit-frame-pointer -emit-llvm -c ./work.c -o work.bc -lpthread
```

or use wllvm to get the bitcode.
```sh
wllvm -g -O0 -fno-omit-frame-pointer ./work.c -o work -lpthread
extract-bc ./work
```
(If you have any questions on WLLVM, you can see WLLVM's Documentation).

**Perform our static analysis scripts to analyze key points for further scheduling.**

Then use the script `staticAnalysis.sh` to perform the static analysis base on SVF. This will analyze key points for further scheduling (key points are the statements/expressions accessing shared memory locations or containing synchronization primitives): 
```sh
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh work
```

Then you can see the information of key points in the file `ConConfig.work`. 
```sh
work.c:10
work.c:11
work.c:13
work.c:9
```

**Import the information of key points to the environment variable `Con_PATH`, and use the wapper `$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++` to perform instrumentation on the `.bc` file (here can also add AddressSanitizer instrumentation).**
```sh
export Con_PATH=$ROOT_DIR/test/work/ConConfig.work
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -O0 -fno-omit-frame-pointer -fsanitize=address ./work.bc -o work
```

**Expose the bug through PERIOD's systematic testing.**
execute the script `$ROOT_DIR/tool/DBDS/run_PDS.py` for PERIOD's controlled concurrency testing:
```sh
$ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -l ./work
```


**Reproduce the buggy interleavings if found the bugs.**

As described above, use the command like the following to reproduce the memory leaks:
```sh
$ROOT_DIR/tool/DBDS/run_PDS.py -r out_work_1/Errors/000001_0000003_memory-leaks -l ./work
```

As described above, use the command like the following to reproduce the double-free:
```
$ROOT_DIR/tool/DBDS/run_PDS.py -r out_work_1/Errors/000004_0000010_double-free ./work
```

-------------------

#### Run PERIOD with AddressSanitizer

[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) (aka ASan) is a memory error detector for C/C++. PERIOD can be performed with AddressSanitizer.

AddressSanitizer requires to add `-fsanitize=address` into CFLAGS or CXXFLAGS, we provide a llvm wapper. Take [`df.c`](test/doubleFree/df.c), which contains a simple double-free, as an example.

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/doubleFree/
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./df.c -o df.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh df

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/test/doubleFree/ConConfig.df
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./df.c -o df.o
$ clang++ ./df.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o df -lpthread -fsanitize=address -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./df
```

#### Reproduce the Interleaving under a Certain Interleaving

After exectue `run_PDS.py`, it first perform dry run. Then we need to press Enter to continue. Finally, the results should look like following.

```sh
Start Testing!
test 0001: [0, 0, 1, 1]
test 0002: [0, 1, 0, 1]
        [Error Found]: NO.1 double-free
        The interleavings saved in out_df_1/Errors/000001
test 0003: [0, 1, 1, 0]
        [Error Found]: NO.2 double-free
        The interleavings saved in out_df_1/Errors/000002
test 0004: [1, 0, 0, 1]
        [Error Found]: NO.3 double-free
        The interleavings saved in out_df_1/Errors/000003
test 0005: [1, 0, 1, 0]
        [Error Found]: NO.4 double-free
        The interleavings saved in out_df_1/Errors/000004
test 0006: [1, 1, 0, 0]
End Testing!


Total Error Interleavings: 4
Total Timeouts Interleavings: 0
2 status found:
         [0, -6]
0 results found:
--------------------------------------------------
        Last New Find           Total
Round   0                       6
Time    00:00:00.00000          00:00:01.76925

```

From the result, we have found three interleavings that can lead to errors. The interleaving is saved in the folder `out_df_*`. If you want to reproduce a certain interleaving that saved in the folder `out_df_*`, you can perfrom the following command.

```bash
$ROOT_DIR/tool/DBDS/run_PDS.py -r out_df_1/Errors/000001 ./df
```

This command will execute the target program with interleaving `[0, 0, 1, 1]`. Actually, it can trigger a double-free bug.

```sh
=================================================================
==67534==ERROR: AddressSanitizer: attempting double-free on 0x602000000010 in thread T2:
    #0 0x4936fd  (/ConFuzz/test/doubleFree/df+0x4936fd)
    #1 0x4c5aa1  (/ConFuzz/test/doubleFree/df+0x4c5aa1)
    #2 0x7f586093e6b9  (/lib/x86_64-linux-gnu/libpthread.so.0+0x76b9)
    #3 0x7f585f9c74dc  (/lib/x86_64-linux-gnu/libc.so.6+0x1074dc)

0x602000000010 is located 0 bytes inside of 7-byte region [0x602000000010,0x602000000017)
freed by thread T1 here:
    #0 0x4936fd  (/ConFuzz/test/doubleFree/df+0x4936fd)
    #1 0x4c5a01  (/ConFuzz/test/doubleFree/df+0x4c5a01)

previously allocated by thread T0 here:
    #0 0x49397d  (/ConFuzz/test/doubleFree/df+0x49397d)
    #1 0x4c5baf  (/ConFuzz/test/doubleFree/df+0x4c5baf)
    #2 0x7f585f8e083f  (/lib/x86_64-linux-gnu/libc.so.6+0x2083f)

Thread T2 created by T0 here:
    #0 0x47e10a  (/ConFuzz/test/doubleFree/df+0x47e10a)
    #1 0x4c5c39  (/ConFuzz/test/doubleFree/df+0x4c5c39)
    #2 0x7f585f8e083f  (/lib/x86_64-linux-gnu/libc.so.6+0x2083f)
NULL 1

Thread T1 created by T0 here:
    #0 0x47e10a  (/ConFuzz/test/doubleFree/df+0x47e10a)
    #1 0x4c5c16  (/ConFuzz/test/doubleFree/df+0x4c5c16)
    #2 0x7f585f8e083f  (/lib/x86_64-linux-gnu/libc.so.6+0x2083f)

SUMMARY: AddressSanitizer: double-free (/ConFuzz/test/doubleFree/df+0x4936fd)
==67534==ABORTING
```

#### Test without AddressSanitizer

Before you use the tool, we suggest that you first use a simple example ([`increase_double.c`](test/increase_double/increase_double.c)) provided by us to determine whether the tool can work normally.

Please try to perform following command:

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/increase_double
$ ./cleanDIR.sh
$ clang++ -g -emit-llvm -c ./increase_double.cpp -o increase_double.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh increase_double

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/test/increase_double/ConConfig.increase_double
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./increase_double.cpp -o increase_double

# perform PDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./increase_double
```

Then you will see that we find all ten different results.


#### Test with ThreadSanitizer

[ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) (aka TSan) is a fast data race detector for C/C++ and Go. DBDS can be performed with ThreadSanitizer.

ThreadSanitizer requires to add `-fsanitize=thread -fPIE -pie` into `CFLAGS` or `CXXFLAGS`, we provide a llvm wapper. Take [`increase_double.c`](test/increase_double/increase_double.c), which contains a simple double-free, as an example.

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/increase_double
$ ./cleanDIR.sh
$ clang++ -g -emit-llvm -c ./increase_double.cpp -o increase_double.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh increase_double

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/test/increase_double/ConConfig.increase_double
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=thread -fPIE -pie ./increase_double.cpp -o increase_double

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./increase_double
```