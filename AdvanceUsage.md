
#### Test with AddressSanitizer

[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) (aka ASan) is a memory error detector for C/C++. DBDS can be  performed with AddressSanitizer.

AddressSanitizer requires to add `-fsanitize=address` into CFLAGS or CXXFLAGS, we provide a llvm wapper. Take [`df.c`](test/doubleFree/df.c), which contains a simple double-free, as an example.

Before you start, you will have to have clang and llvm ready; use `pip3`
to have the `numpy` module install.
In addition, please download and install the C++ Boost library; therefore
you can compile and get the `dbds-clang-fast` instrumentation tool under
the `tool/staticAnalysis/DBDS-INSTRU` directory.

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

ThreadSanitizer requires to add `-fsanitize=thread -fPIE -pie` into CFLAGS or CXXFLAGS, we provide a llvm wapper. Take [`increase_double.c`](test/increase_double/increase_double.c), which contains a simple double-free, as an example.

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