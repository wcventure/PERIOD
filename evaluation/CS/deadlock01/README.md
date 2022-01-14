# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/deadlock01
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./deadlock01_bad.c -o deadlock01_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh deadlock01_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/deadlock01/ConConfig.deadlock01_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./deadlock01_bad.bc -o deadlock01_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./deadlock01_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0, 0], [1, 1, 1]]
test 0002: [[1, 1, 1], [0, 0, 0]]
...
```

The DeadLock failure:

```sh
Timeout = 5
```
