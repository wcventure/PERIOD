# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/carter01
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./carter01_bad.c -o carter01_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh carter01_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/carter01/ConConfig.carter01_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./carter01_bad.bc -o carter01_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./carter01_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0, 0, 0, 0], [1, 1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1, 1], [0, 0, 0, 0, 0]]
test 0003: [[0], [1, 1, 1, 1, 1], [0, 0, 0, 0]]
test 0004: [[1], [0, 0, 0, 0, 0], [1, 1, 1, 1]]
test 0005: [[0, 0], [1, 1, 1, 1, 1], [0, 0, 0]]
	[Timeout 5]: NO.1
	under interleavings saved in out_carter01_bad_1/Timeouts/000001
...
```

The DeadLock failure:

```sh
Timeout = 5
```
