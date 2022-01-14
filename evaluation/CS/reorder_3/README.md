# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/reorder_3
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./reorder_3_bad.c -o reorder_3_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh reorder_3_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/reorder_3/ConConfig.reorder_3_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./reorder_3_bad.bc -o reorder_3_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./reorder_3_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[2], [0, 0], [1, 1]]
test 0002: [[2], [1, 1], [0, 0]]
test 0003: [[0, 0], [2], [1, 1]]
...
```

The Assertion failure:

```sh
reorder_3_bad: reorder_bad.c:80: void *checkThread(void *): Assertion `0' failed.
```
