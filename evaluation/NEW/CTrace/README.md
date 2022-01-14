# About CTrace

See http://ctrace.sourceforge.net/

# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
# then complie the instrumented program
cd $ROOT_DIR/evaluation/NEW/CTrace
./cleanDIR.sh
CC=wllvm CFLAGS="-g -O0" make
cd $ROOT_DIR/evaluation/NEW/CTrace/obj
extract-bc ctrace.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh ctrace.o

export Con_PATH=$ROOT_DIR/evaluation/NEW/CTrace/obj/ConConfig.ctrace.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -c ./ctrace.o.bc -o ctrace.o -lpthread -ldl -fsanitize=thread

cd $ROOT_DIR/evaluation/NEW/CTrace
ar rus lib/libctrace.a obj/*.o

cd test
wllvm -I../inc -L../lib -g -O0 -c main.c
extract-bc ./main.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -O0 -c ./main.o.bc -o main.o
clang++ -o test -I../inc -L../lib -g  main.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o ../obj/ctrace.o -lpthread -ldl -fsanitize=thread

# perform DBDS
$ cd test
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 3 ./test
```

Then you will get the results.

```sh
Start Testing!
Targeting bugs that bug depth = 1 . Iterate for 2 periods
test 0001: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
Targeting bugs that bug depth = 2 . Iterate for 3 periods
test 0003: [[0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0004: [[0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0005: [[0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
        [Error Found]: NO.1 data-race
        The interleavings saved in out_test_8/Errors/000001_0000005_data-race
test 0006: [[0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
        [Error Found]: NO.2 data-race
        The interleavings saved in out_test_8/Errors/000002_0000006_data-race
test 0007: [[0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0008: [[0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0]]
        [Error Found]: NO.3 data-race
        The interleavings saved in out_test_8/Errors/000003_0000008_data-race
test 0009: [[0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0]]
test 0010: [[0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0]]
test 0011: [[0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0]]
test 0012: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0]]
test 0013: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0]]
test 0014: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0]]
test 0015: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0]]
test 0016: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0]]
test 0017: [[1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]]
...
```

In this program, we can find two bugs, one is invaild address dereference, another is data race.
The ocurrence of invaild address dereference is probabilistic, as it rely on the hash value of probabilistic `pthread_self()` id.
The data race is more easier to be detected by using PDS.



The invaild address dereference reported by ASAN:
```
ThreadSanitizer:DEADLYSIGNAL
==4457==ERROR: ThreadSanitizer: SEGV on unknown address 0x000100000001 (pc 0x0000004c128e bp 0x7ffffd43ecb0 sp 0x7ffffd43ec90 T4457)
==4457==The signal is caused by a READ memory access.
    #0 trc_thread /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:225 (test+0x4c128e)
    #1 trc_add_thread /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:264 (test+0x4c14c0)
    #2 main /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:25 (test+0x4b5354)
    #3 __libc_start_main ??:? (libc.so.6+0x270b2)
    #4 _start ??:? (test+0x41e5fd)

ThreadSanitizer can not provide additional info.
SUMMARY: ThreadSanitizer: SEGV /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:225 in trc_thread
==4457==ABORTING
```

Another invaild address dereference reported by ASAN:
```
AddressSanitizer:DEADLYSIGNAL
=================================================================
==17670==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000001 (pc 0x0000004d3337 bp 0x7ffcb3196430 sp 0x7ffcb3196400 T0)
==17670==The signal is caused by a READ memory access.
==17670==Hint: address points to the zero page.
^[[A
    #0 0x4d3337 in trc_remove_thread /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:317:31
    #1 0x4c7194 in main /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:50:2
    #2 0x7fd8894980b2 in __libc_start_main /build/glibc-eX1tMB/glibc-2.31/csu/../csu/libc-start.c:308:16
    #3 0x41c69d in _start (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/test+0x41c69d)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:317:31 in trc_remove_thread
==17670==ABORTING
```

The Data Race report by TSAN:

```sh
==================
WARNING: ThreadSanitizer: data race (pid=2793)
  Write of size 8 at 0x7b0400000000 by thread T2:
    #0 free /home/brian/src/final/llvm-project/compiler-rt/lib/tsan/rtl/tsan_interceptors_posix.cpp:707:3 (test+0x424b08)
    #1 tzset_internal /build/glibc-eX1tMB/glibc-2.31/time/tzset.c:401:3 (libc.so.6+0xd51b1)
    #2 trc_trace /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:787:12 (test+0x4c2fe3)
    #3 thread2 /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:79:2 (test+0x4b5972)

  Previous write of size 8 at 0x7b0400000000 by main thread:
    #0 malloc /home/brian/src/final/llvm-project/compiler-rt/lib/tsan/rtl/tsan_interceptors_posix.cpp:651:5 (test+0x4244b4)
    #1 strdup /build/glibc-eX1tMB/glibc-2.31/string/strdup.c:42:15 (libc.so.6+0xa250e)
    #2 trc_trace /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:787:12 (test+0x4c2fe3)
    #3 main /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:42:2 (test+0x4b54b9)

  Thread T2 (tid=2796, running) created by main thread at:
    #0 pthread_create /home/brian/src/final/llvm-project/compiler-rt/lib/tsan/rtl/tsan_interceptors_posix.cpp:962:3 (test+0x425d7b)
    #1 main /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:34:5 (test+0x4b541b)

SUMMARY: ThreadSanitizer: data race /build/glibc-eX1tMB/glibc-2.31/time/tzset.c:401:3 in tzset_internal
==================
==================
WARNING: ThreadSanitizer: data race (pid=2793)
  Write of size 8 at 0x7fce1f5b01c0 by thread T2:
    #0 localtime /home/brian/src/final/llvm-project/compiler-rt/lib/tsan/../sanitizer_common/sanitizer_common_interceptors.inc:1330:3 (test+0x4327f7)
    #1 trc_trace /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:787:12 (test+0x4c2fe3)
    #2 thread2 /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:79:2 (test+0x4b5972)

  Previous write of size 8 at 0x7fce1f5b01c0 by main thread:
    #0 localtime /home/brian/src/final/llvm-project/compiler-rt/lib/tsan/../sanitizer_common/sanitizer_common_interceptors.inc:1330:3 (test+0x4327f7)
    #1 trc_trace /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:787:12 (test+0x4c2fe3)
    #2 main /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:42:2 (test+0x4b54b9)

  Location is global '??' at 0x7fce1f3bf000 (libc.so.6+0x0000001f11c0)

  Thread T2 (tid=2796, running) created by main thread at:
    #0 pthread_create /home/brian/src/final/llvm-project/compiler-rt/lib/tsan/rtl/tsan_interceptors_posix.cpp:962:3 (test+0x425d7b)
    #1 main /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/test/main.c:34:5 (test+0x4b541b)

SUMMARY: ThreadSanitizer: data race /mnt/e/DockerImage/ConFuzz/evaluation/NEW/CTrace/src/ctrace.c:787:12 in trc_trace
==================
```

# README.txt

This is the README.txt file for the ctrace library.

You will find more in-depth documentation in the doc directory:
doc/manual.html

For up to date documentation and the latest release of ctrace go to:
http://ctrace.sourcefoge.net

This software uses the excellent Semaphore library written by 
Tom Wagner. Redistribution of this software requires you to 
provide a similar reference to this fact.


QUICKSTART FOR USERS
--------------------
To quickly use the ctrace library:
1. Go to ctrace base directory.
2. Run the make utility: 
	$ make
3. Copy the file lib/libctrace.a into your local lib directory.
4. Copy the header files "inc/ctrace.h", "inc/ctrace_types.h" to 
   your local header directory. 
5. Include "ctrace.h" in your source file and use its macro calls.
6. Link the library libctrace.a into your exe


QUICKSTART FOR DEVELOPERS
-------------------------
Following is a description of the directories and files and how to use them:

Files
-----
src/*.c		-> ctrace library source files
src/*.d		-> ctrace library source file dependencies(generated by make)
src/makefile	-> creates ctrace library object files

Actions
-------
$make		-> compile src/*.c files, put objs into obj/*.o 
$make dbug=1	-> compile src/*.c files, put debuggable objs into dobj/*.o 
 

Files
-----
test/*.c	-> ctrace test harness source files
test/*.d	-> test harness source file dependencies(generated by make)
src/makefile	-> creates test harness exe 

Actions
-------
$make		-> compile test/*.c files into test/*.o files, link
		   with obj/*.o files to form test harness executable 
$make dbug=1	-> compile test/*.c files into debuggable test/*.o files, 
		   link with dobj/*.o files to form debuggable test harness
			

Files
-----
makefile	-> run src/makefile, then link objs into ctrace library

Actions
-------
$make		-> run src/makefile, link obj/*.o into lib/libctrace.a
$make dbug=1	-> run src/makefile with dbug=1 option, link dobj/*.o
   		   into debuggable dlib/libctrace.a

N.B. - makefiles mostly use pattern rules, so should not require modification


