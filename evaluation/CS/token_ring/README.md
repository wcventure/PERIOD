# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/token_ring
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./token_ring_bad.c -o token_ring_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh token_ring_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/token_ring/ConConfig.token_ring_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./token_ring_bad.bc -o token_ring_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./token_ring_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0, 0], [1, 1, 1], [2, 2, 2], [3, 3, 3]]
test 0002: [[0, 0, 0], [1, 1, 1], [3, 3, 3], [2, 2, 2]]
test 0003: [[0, 0, 0], [2, 2, 2], [1, 1, 1], [3, 3, 3]]
...
```

The Assertion failure:

```sh
token_ring_bad: ./token_ring_bad.c:45: void *t4(void *): Assertion `x1 == x2 && x2 == x3' failed
```
