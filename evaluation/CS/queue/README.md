# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/queue
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./queue_bad.c -o queue_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh queue_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/queue/ConConfig.queue_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./queue_bad.bc -o queue_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./queue_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0002: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]]
...
```

The Assertion failure:

```sh
queue_bad: ./queue_bad.c:122: void *t2(void *): Assertion `dequeue(&queue)==stored_elements[i]' failed.
```
