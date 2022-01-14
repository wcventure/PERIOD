# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
cd $ROOT_DIR/test/work
./cleanDIR.sh
clang -g -emit-llvm -c ./work.c -o work.bc

# perform static analysis
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh work

# complie the instrumented program with ASAN
export Con_PATH=$ROOT_DIR/test/work/ConConfig.work
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./work.c -o work.o
clang++ ./work.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o work -lpthread -fsanitize=address -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -l ./work
```

Then you will get the results.

```sh
Start Testing!

Targeting bugs that bug depth = 1 . Iterate for 2 periods
test 0001: [[0, 0, 0, 0], [1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1], [0, 0, 0, 0]]

Targeting bugs that bug depth = 2 . Iterate for 3 periods
test 0003: [[0], [1, 1, 1, 1], [0, 0, 0]]
        [Error Found]: NO.1 memory leaks
        The interleavings saved in out_work_22/Errors/000001_0000003_memory-leaks
test 0004: [[1], [0, 0, 0, 0], [1, 1, 1]]
        [Error Found]: NO.2 memory leaks
        The interleavings saved in out_work_22/Errors/000002_0000004_memory-leaks
test 0005: [[0, 0], [1, 1, 1, 1], [0, 0]]
test 0006: [[1, 1], [0, 0, 0, 0], [1, 1]]
test 0007: [[0, 0, 0], [1, 1, 1, 1], [0]]
test 0008: [[1, 1, 1], [0, 0, 0, 0], [1]]

Targeting bugs that bug depth = 3 . Iterate for 4 periods
test 0009: [[0], [1], [0, 0, 0], [1, 1, 1]]
        [Error Found]: NO.3 memory leaks
        The interleavings saved in out_work_22/Errors/000003_0000009_memory-leaks
test 0010: [[1], [0], [1, 1, 1], [0, 0, 0]]
        [Error Found]: NO.4 memory leaks
        The interleavings saved in out_work_22/Errors/000004_0000010_memory-leaks
test 0011: [[0], [1, 1], [0, 0, 0], [1, 1]]
        [Error Found]: NO.5 double-free
        The interleavings saved in out_work_22/Errors/000005_0000011_double-free
test 0012: [[1], [0, 0], [1, 1, 1], [0, 0]]
        [Error Found]: NO.6 double-free
        The interleavings saved in out_work_22/Errors/000006_0000012_double-free
```

The ASAN output for doublefree:
```sh
=================================================================
==23575==ERROR: AddressSanitizer: attempting double-free on 0x602000001010 in thread T2:
    #0 0x494a9d  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x494a9d)
    #1 0x4c6e24  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x4c6e24)
    #2 0x7feed10fd608  (/lib/x86_64-linux-gnu/libpthread.so.0+0x9608)
    #3 0x7feed0cc6292  (/lib/x86_64-linux-gnu/libc.so.6+0x122292)

0x602000001010 is located 0 bytes inside of 10-byte region [0x602000001010,0x60200000101a)
freed by thread T1 here:
    #0 0x494a9d  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x494a9d)
    #1 0x4c6e24  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x4c6e24)

previously allocated by thread T2 here:
    #0 0x494d1d  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x494d1d)
    #1 0x4c6dbd  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x4c6dbd)

Thread T2 created by T0 here:
    #0 0x47faca  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x47faca)
    #1 0x4c6f70  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x4c6f70)
    #2 0x7feed0bcb0b2  (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)

Thread T1 created by T0 here:
    #0 0x47faca  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x47faca)
    #1 0x4c6f4d  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x4c6f4d)
    #2 0x7feed0bcb0b2  (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)

SUMMARY: AddressSanitizer: double-free (/mnt/e/DockerImage/ConFuzz/test/work/work+0x494a9d)
==23575==ABORTING
```

Use `addr2line -e ./work 0x4c6e24` to see the debug info
```
/workdir/ConFuzz/test/work/./work.c:13
```


The ASAN output for memory leak:
```sh
=================================================================
==25158==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 10 byte(s) in 1 object(s) allocated from:
    #0 0x494d1d  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x494d1d)
    #1 0x4c6dbd  (/mnt/e/DockerImage/ConFuzz/test/work/work+0x4c6dbd)

SUMMARY: AddressSanitizer: 10 byte(s) leaked in 1 allocation(s).
```

Use `addr2line -e ./work 0x494d1d` to see the debug info
```
/mnt/e/DockerImage/ConFuzz/test/work/./work.c:10
```