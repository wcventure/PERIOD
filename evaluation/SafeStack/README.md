# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/SafeStack
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./SafeStack.c -o SafeStack.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh SafeStack

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/SafeStack/ConConfig.SafeStack
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./SafeStack.bc -o SafeStack -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 5 -z ./SafeStack
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