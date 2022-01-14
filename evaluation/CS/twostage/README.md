# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/twostage
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./twostage_bad.c -o twostage_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh twostage_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/twostage/ConConfig.twostage_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./twostage_bad.bc -o twostage_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./twostage_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0, 0, 0], [1, 1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1, 1], [0, 0, 0, 0]]
...
```

The Assertion failure:

```sh
twostage_bad: ./twostage_bad.c:48: void *funcB(void *): Assertion `0' failed.
```
