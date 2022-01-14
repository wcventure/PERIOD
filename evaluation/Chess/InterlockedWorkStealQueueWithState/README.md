# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Chess/InterlockedWorkStealQueueWithState
$ ./cleanDIR.sh
$ clang++ InterlockedWorkStealQueueWithState.cpp -std=c++0x -pthread -g -O0 -emit-llvm -c -o InterlockedWorkStealQueueWithState.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh InterlockedWorkStealQueueWithState

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Chess/InterlockedWorkStealQueueWithState/ConConfig.InterlockedWorkStealQueueWithState
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ InterlockedWorkStealQueueWithState.cpp -std=c++0x -pthread -g -O0 -ldl -o InterlockedWorkStealQueueWithState

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./InterlockedWorkStealQueueWithState 2 4 2
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

You can find some schedules that can cause Assertion failure.


```sh
WorkStealQueue Test: 2 stealers, 4 items, and 1 stealAttempts
StateWorkStealQueue: ./WorkStealQueueWithState.h:79: THE::ActivationRecord &THE::State::Peek(int): Assertion `size > 0' failed.
```

Or trigger an exception:

```sh
WorkStealQueue Test: 2 stealers, 4 items, and 1 stealAttempts
terminate called after throwing an instance of 'std::out_of_range'
  what():  vector::_M_range_check: __n (which is 2) >= this->size() (which is 2)
```