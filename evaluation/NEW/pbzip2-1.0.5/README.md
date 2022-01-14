# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src
$ ./cleanDIR.sh
$ CXX=wllvm++ make
$ extract-bc ./pbzip2

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh pbzip2

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5/ConConfig.pbzip2
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=address -O2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_STACKSIZE_CUSTOMIZATION -pthread -D_POSIX_PTHREAD_SEMANTICS  pbzip2.cpp BZ2StreamScanner.cpp ErrorContext.cpp -o pbzip2 -lbz2 -lpthread

# perform DBDS
$ cd $ROOT_DIR/evaluation/CB/pbzip2-1.0.5
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 2 ./pbzip2-1.0.5-src/pbzip2 -k -f -p3 test.tar
```

You can use the script `build.sh` (same as above command, The command `source tool/init_env.sh` is need whenever you open a new terminal)
```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/NEW/pbzip2-1.0.5
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 2 ./pbzip2-1.0.5-src/pbzip2 -k -f -p3 test.tar
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

The ASAN output of heap-buffer-overflow:

```sh
=================================================================
==15805==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6020000000a8 at pc 0x0000004e2a4a bp 0x7fc7938f8e20 sp 0x7fc7938f8e18
READ of size 4 at 0x6020000000a8 thread T2
    #0 0x4e2a49  (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2+0x4e2a49)
    #1 0x7fc7971f8608  (/lib/x86_64-linux-gnu/libpthread.so.0+0x9608)
    #2 0x7fc796dc1292  (/lib/x86_64-linux-gnu/libc.so.6+0x122292)

0x6020000000a8 is located 8 bytes to the right of 16-byte region [0x602000000090,0x6020000000a0)
allocated by thread T0 here:
    #0 0x4d37bd  (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2+0x4d37bd)
    #1 0x4ed1f0  (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2+0x4ed1f0)

Thread T2 created by T0 here:
    #0 0x48edda  (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2+0x48edda)
    #1 0x4e98c9  (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2+0x4e98c9)
    #2 0x7fc796cc60b2  (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)

SUMMARY: AddressSanitizer: heap-buffer-overflow (/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2+0x4e2a49)
Shadow bytes around the buggy address:
  0x0c047fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff8000: fa fa 00 fa fa fa 04 fa fa fa 04 fa fa fa 00 fa
=>0x0c047fff8010: fa fa 00 00 fa[fa]fa fa fa fa fa fa fa fa fa fa
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
==15805==ABORTING
```

Use `addr2line -e ././pbzip2-1.0.5-src/pbzip2 0x4e20b7` to see the debug info
```
/mnt/e/DockerImage/ConFuzz/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2.cpp:811
```