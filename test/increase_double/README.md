# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/increase_double
$ ./cleanDIR.sh
$ clang++ -g -emit-llvm -c ./increase_double.cpp -o increase_double.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh increase_double

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/test/increase_double/ConConfig.increase_double
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./increase_double.cpp -o increase_double

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./increase_double
```

Then you will see that we find all ten different results.

```sh
Total Error Interleavings: 0
1 status found:
         [0]
10 results found:
         b'finished 1\nfinished 2\nx = 16\n'
         b'finished 1\nfinished 2\nx = 12\n'
         b'finished 1\nfinished 2\nx = 10\n'
         b'finished 2\nfinished 1\nx = 9\n'
         b'finished 1\nfinished 2\nx = 8\n'
         b'finished 1\nfinished 2\nx = 6\n'
         b'finished 2\nfinished 1\nx = 5\n'
         b'finished 1\nfinished 2\nx = 4\n'
         b'finished 2\nfinished 1\nx = 3\n'
         b'finished 2\nfinished 1\nx = 2\n'
--------------------------------------------------
        Last New Find           Total
Round   0                       10
Time    00:00:00.00000          00:00:01.98619
```
