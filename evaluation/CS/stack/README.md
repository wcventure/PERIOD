# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/stack
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./stack_bad.c -o stack_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh stack_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/stack/ConConfig.stack_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./stack_bad.bc -o stack_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./stack_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0002: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
...
```

The Assertion failure:

```sh
stack_bad: ./stack_bad.c:89: void *t2(void *): Assertion `pop(arr)!=UNDERFLOW' failed.
```
