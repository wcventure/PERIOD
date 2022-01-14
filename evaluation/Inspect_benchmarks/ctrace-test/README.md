# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Inspect_benchmarks/ctrace-test
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./ctrace-test.c -o ctrace-test.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh ctrace-test

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Inspect_benchmarks/ctrace-test/ConConfig.ctrace-test
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./ctrace-test.bc -o ctrace-test -lpthread -ldl -fsanitize=thread

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 3 ./ctrace-test 2
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
```