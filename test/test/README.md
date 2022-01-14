# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
cd $ROOT_DIR/test/test
./cleanDIR.sh
clang -g -emit-llvm -c ./test.c -o test.bc

# perform static analysis
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh test

# complie the instrumented program with ASAN
export Con_PATH=$ROOT_DIR/test/test/ConConfig.test
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./test.c -o test.o
clang++ ./test.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o test -lpthread -fsanitize=address -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 2 ./test
```

Then you will get the results.

```sh
Start Testing!
Targeting bugs that bug depth = 1 . Iterate for 2 periods
test 0001: [[0, 0, 0, 0, 0, 0], [1, 1]]
test 0002: [[1, 1], [0, 0, 0, 0, 0, 0]]
Targeting bugs that bug depth = 2 . Iterate for 3 periods
test 0003: [[0], [1, 1], [0, 0, 0, 0, 0]]
        [Error Found]: NO.1 assertion failed
        The interleavings saved in out_test_8/Errors/000001_0000003_assertion-failed
test 0004: [[0, 0], [1, 1], [0, 0, 0, 0]]
        [Error Found]: NO.2 assertion failed
        The interleavings saved in out_test_8/Errors/000002_0000004_assertion-failed
test 0005: [[0, 0, 0], [1, 1], [0, 0, 0]]
        [Error Found]: NO.3 assertion failed
        The interleavings saved in out_test_8/Errors/000003_0000005_assertion-failed
...
```

The assertion fail:

```
test: ./test.c:30: int main(int, char **): Assertion `worker==1' failed.
```
