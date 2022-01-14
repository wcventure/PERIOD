# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Parsec-2.0/streamcluster
$ ./cleanDIR.sh
$ CXX=wllvm++ make
$ extract-bc ./streamcluster

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh streamcluster

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Parsec-2.0/streamcluster/ConConfig.streamcluster
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./streamcluster.bc -o streamcluster -lpthread -ldl -fsanitize=address

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 2 ./streamcluster 2 5 1 10 10 5 none output.txt 2
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