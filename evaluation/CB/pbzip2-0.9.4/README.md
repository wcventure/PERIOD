# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src
$ ./cleanDIR.sh
$ CXX=wllvm++ make
$ extract-bc ./pbzip2

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh pbzip2

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/ConConfig.pbzip2
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=address ./pbzip2.cpp -o pbzip2 -lpthread -ldl -lbz2

# perform DBDS
$ cd $ROOT_DIR/evaluation/CB/pbzip2-0.9.4
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 2 ./pbzip2-0.9.4-src/pbzip2 -k -f -p3 test.tar
```

You can use the script `build.sh` (same as above command, The command `source tool/init_env.sh` is need whenever you open a new terminal)
```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/CB/pbzip2-0.9.4
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 2 ./pbzip2-0.9.4-src/pbzip2 -k -f -p3 test.tar
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```


The ASAN output of use-after-free:

```sh
Parallel BZIP2 v0.9.4 - by: Jeff Gilchrist [http://compression.ca]
[Aug. 30, 2005]             (uses libbzip2  by Julian Seward)

** This is a BETA version - Use at your own risk! **

         # CPUs: 3
 BWT Block Size: 900k
File Block Size: 900k
-------------------------------------------
         File #: 1 of 1
     Input Name: test.tar
    Output Name: test.tar.bz2

     Input Size: 81920 bytes
Compressing data...
    Output Size: 17086 bytes
-------------------------------------------
=================================================================
==14452==ERROR: AddressSanitizer: heap-use-after-free on address 0x6070000000c0 at pc 0x56496ddbb11c bp 0x7efe5aafecd0 sp 0x7efe5aafecc0
READ of size 8 at 0x6070000000c0 thread T1
    #0 0x56496ddbb11b in consumer(void*) (/concurrency-bugs/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x711b)
    #1 0x7efe5e9456da in start_thread (/lib/x86_64-linux-gnu/libpthread.so.0+0x76da)
    #2 0x7efe5debda3e in __clone (/lib/x86_64-linux-gnu/libc.so.6+0x121a3e)

0x6070000000c0 is located 48 bytes inside of 72-byte region [0x607000000090,0x6070000000d8)
freed by thread T0 here:
    #0 0x7efe5ec3e9c8 in operator delete(void*, unsigned long) (/usr/lib/x86_64-linux-gnu/libasan.so.4+0xe19c8)
    #1 0x56496ddb96c6 in main (/concurrency-bugs/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x56c6)
    #2 0x327a622d  (<unknown module>)

previously allocated by thread T0 here:
    #0 0x7efe5ec3d448 in operator new(unsigned long) (/usr/lib/x86_64-linux-gnu/libasan.so.4+0xe0448)
    #1 0x56496ddbc0ac in queueInit(int) (/concurrency-bugs/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x80ac)

Thread T1 created by T0 here:
    #0 0x7efe5eb94d2f in __interceptor_pthread_create (/usr/lib/x86_64-linux-gnu/libasan.so.4+0x37d2f)
    #1 0x56496ddb9031 in main (/concurrency-bugs/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x5031)

SUMMARY: AddressSanitizer: heap-use-after-free (/concurrency-bugs/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x711b) in consumer(void*)
Shadow bytes around the buggy address:
  0x0c0e7fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c0e7fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c0e7fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c0e7fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c0e7fff8000: fa fa fa fa 00 00 00 00 00 00 00 00 00 fa fa fa
=>0x0c0e7fff8010: fa fa fd fd fd fd fd fd[fd]fd fd fa fa fa fa fa
  0x0c0e7fff8020: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c0e7fff8030: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c0e7fff8040: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c0e7fff8050: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c0e7fff8060: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==14452==ABORTING
```

Use `addr2line -e ././pbzip2-0.9.4-src/pbzip2 0x76db` to see the debug info
```
/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/./pbzip2.cpp:898
```


The ASAN output of Null Pointer Deference:

```sh
Parallel BZIP2 v0.9.4 - by: Jeff Gilchrist [http://compression.ca]
[Aug. 30, 2005]             (uses libbzip2  by Julian Seward)

** This is a BETA version - Use at your own risk! **

         # CPUs: 2
 BWT Block Size: 900k
File Block Size: 900k
-------------------------------------------
         File #: 1 of 1
     Input Name: test.tar
    Output Name: test.tar.bz2

     Input Size: 81920 bytes
Compressing data...
    Output Size: 17086 bytes
-------------------------------------------
AddressSanitizer:DEADLYSIGNAL
=================================================================
==28187==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000010 (pc 0x7f118935ffc4 bp 0x7f1186261e70 sp 0x7f1186261cb8 T1)
==28187==The signal is caused by a READ memory access.
==28187==Hint: address points to the zero page.
    #0 0x7f118935ffc4  (/lib/x86_64-linux-gnu/libpthread.so.0+0xbfc4)
    #1 0x4e3174  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4e3174)
    #2 0x7f118935d608  (/lib/x86_64-linux-gnu/libpthread.so.0+0x9608)
    #3 0x7f1188f13292  (/lib/x86_64-linux-gnu/libc.so.6+0x122292)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV (/lib/x86_64-linux-gnu/libpthread.so.0+0xbfc4)
Thread T1 created by T0 here:
    #0 0x48edaa  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x48edaa)
    #1 0x4e6cb7  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4e6cb7)
    #2 0x7f1188e180b2  (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)

==28187==ABORTING
```

Use `addr2line -e ././pbzip2-0.9.4-src/pbzip2 0x4e29e0` to see the debug info
```
/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/./pbzip2.cpp:890
```


The ASAN output of heap-buffer-overflow:

```sh
Parallel BZIP2 v0.9.4 - by: Jeff Gilchrist [http://compression.ca]
[Aug. 30, 2005]             (uses libbzip2  by Julian Seward)

** This is a BETA version - Use at your own risk! **

         # CPUs: 2
 BWT Block Size: 900k
File Block Size: 900k
-------------------------------------------
         File #: 1 of 1
     Input Name: test.tar
    Output Name: test.tar.bz2

     Input Size: 81920 bytes
Compressing data...
=================================================================
==27624==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000088 at pc 0x0000004e29e1 bp 0x7f0896b8ae20 sp 0x7f0896b8ae18
READ of size 4 at 0x602000000088 thread T3
    #0 0x4e29e0  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4e29e0)
    #1 0x7f089acb4608  (/lib/x86_64-linux-gnu/libpthread.so.0+0x9608)
    #2 0x7f089a86a292  (/lib/x86_64-linux-gnu/libc.so.6+0x122292)

0x602000000088 is located 8 bytes to the right of 16-byte region [0x602000000070,0x602000000080)
allocated by thread T0 here:
    #0 0x4d378d  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4d378d)
    #1 0x4e9ee0  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4e9ee0)

Thread T3 created by T0 here:
    #0 0x48edaa  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x48edaa)
    #1 0x4e6cf9  (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4e6cf9)
    #2 0x7f089a76f0b2  (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)

SUMMARY: AddressSanitizer: heap-buffer-overflow (/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/pbzip2+0x4e29e0)
Shadow bytes around the buggy address:
  0x0c047fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff8000: fa fa 00 00 fa fa 00 fa fa fa 00 fa fa fa 00 00
=>0x0c047fff8010: fa[fa]fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8020: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8030: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8040: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8050: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8060: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
  Shadow gap:              cc
==27624==ABORTING
```

Use `addr2line -e ././pbzip2-0.9.4-src/pbzip2 0x4e29e0` to see the debug info
```
/mnt/e/DockerImage/ConFuzz/evaluation/CB/pbzip2-0.9.4/pbzip2-0.9.4-src/./pbzip2.cpp:705
```