# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/circular_buffer
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./circular_buffer_bad.c -o circular_buffer_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh circular_buffer_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/circular_buffer/ConConfig.circular_buffer_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./circular_buffer_bad.bc -o circular_buffer_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./circular_buffer_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0003: [[0], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]
test 0003 (retest)
	[Error Found]: NO.1 assertion failed
	The interleavings saved in out_circular_buffer_bad_1/Errors/000001_0000003_assertion-failed
...
```

The Assertion failure:

```sh
circular_buffer_bad: ./circular_buffer_bad.c:84: void *t2(void *): Assertion `removeLogElement()==i' failed.
```
