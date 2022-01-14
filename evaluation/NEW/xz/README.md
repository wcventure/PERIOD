# Test

Use the script `build.sh` (The command `source tool/init_env.sh` is need whenever you open a new terminal). Please try to perform following command:

```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/NEW/lbzip2
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -t 5 ./xz-5.2.5/src/xz/.libs/xz -9 -k -T 4 -f $FILE
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```