# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/issue205
$ ./cleanDIR.sh
$ clang++ -g -emit-llvm -c ./issue205.c -o issue205.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh issue205

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/test/issue205/ConConfig.issue205
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./issue205.c -o issue205 -fsanitize=address

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./issue205
```

Then you will see that we find a Null-pointer-dereference bug and use-after-free bug.

```sh
Start Testing!
Targeting bugs that bug depth = 1 . Iterate for 2 periods
test 0001: [[0], [1, 1]]
test 0002: [[1, 1], [0]]
        [Error Found]: NO.1 SEGV
        The interleavings saved in out_issue205_1/Errors/000001_0000002_SEGV
Targeting bugs that bug depth = 2 . Iterate for 3 periods
test 0003: [[1], [0], [1]]
        [Error Found]: NO.2 heap-use-after-free
        The interleavings saved in out_issue205_1/Errors/000002_0000003_heap-use-after-free
End Testing!
```
