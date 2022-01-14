# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/UAF/
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./uaf.c -o uaf.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh uaf

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/test/UAF/ConConfig.uaf
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./uaf.c -o uaf.o
$ clang++ ./uaf.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o uaf -lpthread -fsanitize=address -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./uaf
```

Then you will get the results.

```sh
Start Testing!
test 0001: [0][1, 0, 0]
test 0002: [1][0, 2, 0]
test 0003: [1][0, 0, 2]
test 0004: [1][0, 0, 0]
        [Error Found]: NO.1 heap-use-after-free
        under interleavings saved in out_uaf_1/Errors/000001
End Testing!

Total Error Interleavings: 1
2 status found:
         [1, -6]
1 results found:
         b'start\nuse\nfree\nfinished\n'
--------------------------------------------------
        Last New Find           Total
Round   0                       4
Time    00:00:00.00000          00:00:01.02791
```

The ASAN output:
```sh
=================================================================
==1497==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010 at pc 0x0000004c674a bp 0x7f192790be60 sp 0x7f192790be58
WRITE of size 1 at 0x602000000010 thread T1
    #0 0x4c6749  (/ConcurrencyFuzzer/test/UAF/uaf+0x4c6749)
    #1 0x7f192b5a26da  (/lib/x86_64-linux-gnu/libpthread.so.0+0x76da)
    #2 0x7f192a58088e  (/lib/x86_64-linux-gnu/libc.so.6+0x12188e)

0x602000000010 is located 0 bytes inside of 7-byte region [0x602000000010,0x602000000017)
freed by thread T2 here:
    #0 0x49412d  (/ConcurrencyFuzzer/test/UAF/uaf+0x49412d)
    #1 0x4c655e  (/ConcurrencyFuzzer/test/UAF/uaf+0x4c655e)

previously allocated by thread T0 here:
    #0 0x4943ad  (/ConcurrencyFuzzer/test/UAF/uaf+0x4943ad)
    #1 0x4c6830  (/ConcurrencyFuzzer/test/UAF/uaf+0x4c6830)
    #2 0x7f192a480b96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

Thread T1 created by T0 here:
    #0 0x47f15a  (/ConcurrencyFuzzer/test/UAF/uaf+0x47f15a)
    #1 0x4c6978  (/ConcurrencyFuzzer/test/UAF/uaf+0x4c6978)
    #2 0x7f192a480b96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

Thread T2 created by T0 here:
    #0 0x47f15a  (/ConcurrencyFuzzer/test/UAF/uaf+0x47f15a)
    #1 0x4c699b  (/ConcurrencyFuzzer/test/UAF/uaf+0x4c699b)
    #2 0x7f192a480b96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

SUMMARY: AddressSanitizer: heap-use-after-free (/ConcurrencyFuzzer/test/UAF/uaf+0x4c6749)
Shadow bytes around the buggy address:
  0x0c047fff7fb0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c047fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x0c047fff8000: fa fa[fd]fa fa fa 07 fa fa fa fa fa fa fa fa fa
  0x0c047fff8010: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8020: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8030: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8040: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff8050: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==1497==ABORTING
```
