# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/reorder_4
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./reorder_4_bad.c -o reorder_4_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh reorder_4_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/reorder_4/ConConfig.reorder_4_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./reorder_4_bad.bc -o reorder_4_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./reorder_4_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[3], [0, 0], [1, 1], [2, 2]]
test 0002: [[3], [0, 0], [2, 2], [1, 1]]
test 0003: [[3], [1, 1], [0, 0], [2, 2]]
...
```

The Assertion failure:

```sh
reorder_4_bad: reorder_bad.c:80: void *checkThread(void *): Assertion `0' failed.
```
