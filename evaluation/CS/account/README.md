# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/account
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./account_bad.c -o account_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh account_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/account/ConConfig.account_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./account_bad.bc -o account_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./account_bad
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
produce ....0
total ....0
consume ....0
produce ....1
total ....1
consume ....1
produce ....2
total ....3
consume ....2
total ....6
account_bad: ./account_bad.c:80: int main(void): Assertion `total!=((N*(N+1))/2)' failed.
Aborted
```