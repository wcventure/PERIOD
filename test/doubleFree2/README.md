# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/doubleFree2/
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./df.c -o df.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh df

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/test/doubleFree2/ConConfig.df
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./df.c -o df.o
$ clang++ ./df.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o df -lpthread -fsanitize=address -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./df
```

Then you will get the results.

```sh
Start Testing!
test 0001: [0, 0][1, 0]
test 0002: [0, 2][1, 2]
        [Error Found]: NO.1 double-free
        under interleavings saved in out_df_1/Errors/000001
test 0003: [0, 2][1, 0]
        [Error Found]: NO.2 double-free
        under interleavings saved in out_df_1/Errors/000002
test 0004: [1, 0][0, 2]
        [Error Found]: NO.3 double-free
        under interleavings saved in out_df_1/Errors/000003
test 0005: [1, 2][0, 2]
        [Error Found]: NO.4 double-free
        under interleavings saved in out_df_1/Errors/000004
test 0006: [1, 0][0, 0]
End Testing!

Total Error Interleavings: 4
2 status found:
         [0, -6]
2 results found:
         b'start\nfree 1\nNULL 1\nfree 2\nNULL 2\nfinished\n'
         b'start\nfree 2\nNULL 2\nfree 1\nNULL 1\nfinished\n'
--------------------------------------------------
        Last New Find           Total
Round   0                       6
Time    00:00:00.00000          00:00:01.66995
```

The ASAN output:
```sh
=================================================================
==1395==ERROR: AddressSanitizer: attempting double-free on 0x602000000010 in thread T2:
    #0 0x49412d  (/ConcurrencyFuzzer/test/doubleFree/df+0x49412d)
    #1 0x4c6501  (/ConcurrencyFuzzer/test/doubleFree/df+0x4c6501)
    #2 0x7f55c86e46da  (/lib/x86_64-linux-gnu/libpthread.so.0+0x76da)
    #3 0x7f55c76c288e  (/lib/x86_64-linux-gnu/libc.so.6+0x12188e)

0x602000000010 is located 0 bytes inside of 7-byte region [0x602000000010,0x602000000017)
freed by thread T1 here:
    #0 0x49412d  (/ConcurrencyFuzzer/test/doubleFree/df+0x49412d)
    #1 0x4c6461  (/ConcurrencyFuzzer/test/doubleFree/df+0x4c6461)

previously allocated by thread T0 here:
    #0 0x4943ad  (/ConcurrencyFuzzer/test/doubleFree/df+0x4943ad)
    #1 0x4c660f  (/ConcurrencyFuzzer/test/doubleFree/df+0x4c660f)
    #2 0x7f55c75c2b96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

Thread T2 created by T0 here:
    #0 0x47f15a  (/ConcurrencyFuzzer/test/doubleFree/df+0x47f15a)
    #1 0x4c6699  (/ConcurrencyFuzzer/test/doubleFree/df+0x4c6699)
    #2 0x7f55c75c2b96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

Thread T1 created by T0 here:
    #0 0x47f15a  (/ConcurrencyFuzzer/test/doubleFree/df+0x47f15a)
    #1 0x4c6676  (/ConcurrencyFuzzer/test/doubleFree/df+0x4c6676)
    #2 0x7f55c75c2b96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

SUMMARY: AddressSanitizer: double-free (/ConcurrencyFuzzer/test/doubleFree/df+0x49412d)
==1395==ABORTING
```
