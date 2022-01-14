# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
cd $ROOT_DIR/test/lock
./cleanDIR.sh
clang -g -emit-llvm -c ./lock.c -o lock.bc

# perform static analysis
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh lock

# complie the instrumented program with ASAN
export Con_PATH=$ROOT_DIR/test/work/ConConfig.lock
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -c ./lock.c -o lock.o
clang++ ./lock.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o lock -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 2 -t 1 ./lock
```

Then you will get the results.

```sh
Start Testing!
Targeting bugs that bug depth = 1 . Iterate for 2 periods
test 0001: [[0, 0, 0, 0, 0], [1, 1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1, 1], [0, 0, 0, 0, 0]]
Targeting bugs that bug depth = 2 . Iterate for 3 periods
test 0003: [[0], [1, 1, 1, 1, 1], [0, 0, 0, 0]]
test 0004: [[0, 0], [1, 1, 1, 1, 1], [0, 0, 0]]
test 0005: [[0, 0, 0], [1, 1, 1, 1, 1], [0, 0]]
        [Timeout 1]: NO.1
        under interleavings saved in out_lock_3/Timeouts/000001
test 0006: [[0, 0, 0, 0], [1, 1, 1, 1, 1], [0]]
test 0007: [[1], [0, 0, 0, 0, 0], [1, 1, 1, 1]]
        [Timeout 1]: NO.2
        under interleavings saved in out_lock_3/Timeouts/000002
test 0008: [[1, 1], [0, 0, 0, 0, 0], [1, 1, 1]]
        [Timeout 1]: NO.3
        under interleavings saved in out_lock_3/Timeouts/000003
test 0009: [[1, 1, 1], [0, 0, 0, 0, 0], [1, 1]]
        [Timeout 1]: NO.4
        under interleavings saved in out_lock_3/Timeouts/000004
test 0010: [[1, 1, 1, 1], [0, 0, 0, 0, 0], [1]]
End Testing!
```
