# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Inspect_benchmarks/qsort_mt
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./qsort_mt.c -o qsort_mt.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh qsort_mt

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Inspect_benchmarks/qsort_mt/ConConfig.qsort_mt
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./qsort_mt.bc -o qsort_mt -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 3 ./qsort_mt -n 32 -f 4 -h 3 -v
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
sort error at position 1:  22 > 9
qsort_mt: ./qsort_mt.c:649: int main(int, char **): Assertion `0 && "Failed to verify sorted list!"' failed.
```