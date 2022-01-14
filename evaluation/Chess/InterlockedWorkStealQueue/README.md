# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Chess/InterlockedWorkStealQueue
$ ./cleanDIR.sh
$ clang++ InterlockedWorkStealQueue.cpp -std=c++0x -pthread -g -O0 -emit-llvm -c -o InterlockedWorkStealQueue.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh InterlockedWorkStealQueue

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Chess/InterlockedWorkStealQueue/ConConfig.InterlockedWorkStealQueue
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ InterlockedWorkStealQueue.cpp -std=c++0x -pthread -g -O0 -ldl -o InterlockedWorkStealQueue

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./InterlockedWorkStealQueue 1 4 2
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
InterlockedWorkStealQueue: InterlockedWorkStealQueue.cpp:57: void ObjType::Check(): Assertion `field == 1' failed.
```