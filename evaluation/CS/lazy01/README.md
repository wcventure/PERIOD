# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/lazy01
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./lazy01_bad.c -o lazy01_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh lazy01_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/lazy01/ConConfig.lazy01_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./lazy01_bad.bc -o lazy01_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./lazy01_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0], [1, 1], [2, 2]]
test 0002: [[0, 0], [2, 2], [1, 1]]
test 0003: [[1, 1], [0, 0], [2, 2]]
test 0004: [[1, 1], [2, 2], [0, 0]]
	[Error Found]: NO.1 assertion failed
	The interleavings saved in out_lazy01_bad_1/Errors/000001_0000004_assertion-failed
...
```

The Assertion failure:

```sh
lazy01_bad: ./lazy01_bad.c:27: void *thread3(void *): Assertion `0' failed.
```
