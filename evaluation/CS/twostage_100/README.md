# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/twostage_100
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./twostage_100_bad.c -o twostage_100_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh twostage_100_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/twostage_100/ConConfig.twostage_100_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./twostage_100_bad.bc -o twostage_100_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./twostage_100_bad
```

Then you will get the results.

```sh
Start Testing!
...
```

The Assertion failure:

```sh
twostage_100_bad: twostage_bad.c:48: void *funcB(void *): Assertion `0' failed.
```
