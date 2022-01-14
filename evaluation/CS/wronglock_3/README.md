# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/wronglock_3
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./wronglock_3_bad.c -o wronglock_3_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh wronglock_3_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/wronglock_3/ConConfig.wronglock_3_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./wronglock_3_bad.bc -o wronglock_3_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./wronglock_3_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[1, 1], [2, 2], [3, 3], [0, 0, 0, 0]]
test 0002: [[1, 1], [3, 3], [2, 2], [0, 0, 0, 0]]
test 0003: [[2, 2], [1, 1], [3, 3], [0, 0, 0, 0]]
...
```

The Assertion failure:

```sh
wronglock_3_bad: wronglock_bad.c:23: void *funcA(void *): Assertion `0' failed.
```
