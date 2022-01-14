# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/NEW/pixz-1.0.5
$ ./cleanDIR.sh
$ cd pixz-1.0.5
$ CC=wllvm CXX=wllvm++ CFLAGS="-g" CXXFLAGS="-g" ./configure
$ make
$ cd src
$ extract-bc ./pixz

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh pixz

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/evaluation/NEW/pixz-1.0.5/pixz-1.0.5/src/ConConfig.pixz
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -pthread -Wall -Wno-unknown-pragmas -g -fsanitize=thread -o pixz pixz.bc -lm -larchive -llzma -lm

# perform DBDS
$ cd $ROOT_DIR/evaluation/NEW/pixz-1.0.5
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./pixz-1.0.5/src/pixz -c -k -p 3 test.tar
```

You can use the script `build.sh` (same as above command, The command `source tool/init_env.sh` is need whenever you open a new terminal)
```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd  $ROOT_DIR/evaluation/NEW/pixz-1.0.5
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./pixz-1.0.5/src/pixz -c -k -p 3 test.tar
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```
