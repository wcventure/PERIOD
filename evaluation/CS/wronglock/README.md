# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/wronglock
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./wronglock_bad.c -o wronglock_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh wronglock_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/wronglock/ConConfig.wronglock_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./wronglock_bad.bc -o wronglock_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./wronglock_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [16], [17], [18], [19]]
test 0002: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [16], [17], [19], [18]]
test 0003: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [16], [18], [17], [19]]
test 0004: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [16], [18], [19], [17]]
test 0005: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [16], [19], [17], [18]]
test 0006: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [16], [19], [18], [17]]
test 0007: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [17], [16], [18], [19]]
test 0008: [[0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6], [7, 7], [8, 8], [9, 9], [10], [11], [12], [13], [14], [15], [17], [16], [19], [18]]
...
```

The Assertion failure:

```sh
wronglock_bad: ./wronglock_bad.c:23: void *funcA(void *): Assertion `0' failed.
```
