# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Chess/WorkStealQueue
$ ./cleanDIR.sh
$ clang++ WorkStealQueue.cpp -std=c++0x -pthread -g -O0 -emit-llvm -c -o WorkStealQueue.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh WorkStealQueue

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Chess/WorkStealQueue/ConConfig.WorkStealQueue
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ WorkStealQueue.cpp -std=c++0x -pthread -g -O0 -ldl -o WorkStealQueue

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./WorkStealQueue 1 4 2
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

The assertion failure:
```sh
WorkStealQueue Test: 1 stealers, 4 items, and 2 stealAttempts
WorkStealQueue: WorkStealQueue.cpp:53: void ObjType::Check(): Assertion `field == 1' failed.
```