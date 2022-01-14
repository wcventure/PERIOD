# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Inspect_benchmarks/boundedBuffer
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./boundedBuffer.c -o boundedBuffer.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh boundedBuffer

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Inspect_benchmarks/boundedBuffer/ConConfig.boundedBuffer
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./boundedBuffer.bc -o boundedBuffer -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 3 ./boundedBuffer
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

The Assertion failure:

```sh
boundedBuffer: ./boundedBuffer.c:3: void reach_error(): Assertion `0' failed.
```