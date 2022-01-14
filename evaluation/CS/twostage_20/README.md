# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/twostage_20
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./twostage_20_bad.c -o twostage_20_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh twostage_20_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/twostage_20/ConConfig.twostage_20_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./twostage_20_bad.bc -o twostage_20_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./twostage_20_bad
```

Then you will get the results.

```sh
Start Testing!
...
```

The Assertion failure:

```sh
twostage_20_bad: twostage_bad.c:48: void *funcB(void *): Assertion `0' failed.
```
