# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CB/stringbuffer-jdk1.4
$ ./cleanDIR.sh
$ CXX=wllvm++ make
$ extract-bc ./main

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh main

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/evaluation/CB/stringbuffer-jdk1.4/ConConfig.main
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=address ./main.bc -o main -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./main
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

The ASAN output:
```sh
main: stringbuffer.cpp:54: void StringBuffer::getChars(int, int, char*, int): Assertion `0' failed.
已放弃
```
