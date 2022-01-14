# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/NEW/axel-2.17.10/
$ ./cleanDIR.sh
$ cd $ROOT_DIR/evaluation/NEW/axel-2.17.10/axel-2.17.10/
$ CC=wllvm CXX=wllvm++ CFLAGS="-g -O0" CXXFLAGS="-g -O0" ./configure
$ make
$ extract-bc ./axel

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh axel

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/evaluation/NEW/axel-2.17.10/axel-2.17.10/ConConfig.axel
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=thread -o axel axel.bc -lpthread -ldl -lssl -lcrypto

# perform controlled testing
$ cd $ROOT_DIR/evaluation/NEW/axel-2.17.10/
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -t 100 ./test.sh
```

You can use the script `build.sh` (same as above command, The command `source tool/init_env.sh` is need whenever you open a new terminal)
```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/NEW/axel-2.17.10/
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -t 100 ./test.sh
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```