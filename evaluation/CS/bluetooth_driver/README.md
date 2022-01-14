# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CS/bluetooth_driver
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./bluetooth_driver_bad.c -o bluetooth_driver_bad.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh bluetooth_driver_bad

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CS/bluetooth_driver/ConConfig.bluetooth_driver_bad
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./bluetooth_driver_bad.bc -o bluetooth_driver_bad -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./bluetooth_driver_bad
```

Then you will get the results.

```sh
Start Testing!
test 0001: [[1, 1, 1, 1], [0, 0, 0, 0, 0, 0, 0]]
test 0002: [[0, 0, 0, 0, 0, 0, 0], [1, 1, 1, 1]]
test 0003: [[0], [1, 1, 1, 1], [0, 0, 0, 0, 0, 0]]
test 0004: [[1], [0, 0, 0, 0, 0, 0, 0], [1, 1, 1]]
	[Error Found]: NO.1 assertion failed
	The interleavings saved in out_bluetooth_driver_bad_1/Errors/000001_0000004_assertion-failed
```

The Assertion failure:

```sh
bluetooth_driver_bad: ./bluetooth_driver_bad.c:52: void *BCSP_PnpAdd(DEVICE_EXTENSION *): Assertion `!stopped' failed.
```
