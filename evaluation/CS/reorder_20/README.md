# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/reorder_20
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./reorder_20_bad.c -o reorder_20_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh reorder_20_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/reorder_20/ConConfig.reorder_20_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./reorder_20_bad.bc -o reorder_20_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./reorder_20_bad
```

Then you will get the results.

```sh
...
```

The Assertion failure:

```sh
reorder_20_bad: reorder_bad.c:80: void *checkThread(void *): Assertion `0' failed.
```
