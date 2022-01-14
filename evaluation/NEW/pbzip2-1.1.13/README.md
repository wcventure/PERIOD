# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CB/pbzip2-1.1.13/pbzip2-1.1.13-src
$ ./cleanDIR.sh
$ CXX=wllvm++ make
$ extract-bc ./pbzip2

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh pbzip2

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/evaluation/CB/pbzip2-1.1.13/pbzip2-1.1.13-src/ConConfig.pbzip2
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=address -O2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_STACKSIZE_CUSTOMIZATION -pthread -D_POSIX_PTHREAD_SEMANTICS  pbzip2.cpp BZ2StreamScanner.cpp ErrorContext.cpp -o pbzip2 -lbz2 -lpthread

# perform DBDS
$ cd $ROOT_DIR/evaluation/CB/pbzip2-1.1.13
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 2 ./pbzip2-1.1.13-src/pbzip2 -k -f -p3 test.tar
```

You can use the script `build.sh` (same as above command, The command `source tool/init_env.sh` is need whenever you open a new terminal)
```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/CB/pbzip2-1.1.13
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -t 15 -d 2 ./pbzip2-1.1.13-src/pbzip2 -k -f -p3 test.tar
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

The ASAN output of use-after-free:

```sh

```