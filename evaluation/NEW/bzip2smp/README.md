# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/NEW/bzip2smp
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./bzip2smp.comb.c -o bzip2smp.comb.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh bzip2smp.comb

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/NEW/bzip2smp/ConConfig.bzip2smp.comb
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./bzip2smp.comb.bc -o bzip2smp.comb -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -t 15 -d 3 ./bzip2smp.comb --no-ht -1 -p2 bzip_input output.txt
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