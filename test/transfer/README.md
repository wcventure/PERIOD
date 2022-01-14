# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/transfer
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./transfer.c -o transfer.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh transfer

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/test/transfer/ConConfig.transfer
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -c ./transfer.c -o transfer.o
$ clang++ ./transfer.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -o transfer -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./transfer
```

Then you will see that we find all four different results.

```sh
Total Error Interleavings: 0
1 status found:
         [0]
4 results found:
         b'50,50\n'
         b'130,50\n'
         b'80,20\n'
         b'130,20\n'
--------------------------------------------------
        Last New Find           Total
Round   0                       23
Time    00:00:00.00000          00:00:01.26320
```
