# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/CB/aget-bug2/aget-devel
$ ../cleanDIR.sh
$ CC=wllvm make
$ extract-bc ./aget

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh aget

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/CB/aget-bug2/aget-devel/ConConfig.aget
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./aget.bc -o aget -lpthread -ldl -fsanitize=address

# perform DBDS
$ cd $ROOT_DIR/evaluation/CB/aget-bug2
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 2 ./aget-devel/aget -l dl.txt http://www.gnu.org/licenses/gpl.txt
```

You can use the script `build.sh` (same as above command, The command `source tool/init_env.sh` is need whenever you open a new terminal)
```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/CB/aget-bug2/
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 2 LOOP_UNWINDED_NUM=2 ./aget-devel/aget -l dl.txt http://www.gnu.org/licenses/gpl.txt
```


Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

If the bug is triggered, the program will have an assertion failure:

```sh
verifier: verifier.c:101: read_log: Assertion `total_bwritten == bwritten' failed.
Aborted
```

# Notes

## Summary

A data race and atomicity violation in Aget.

## Details

The bug is triggered when user stop the download process by
typing ctrl-c on the console. The aget program will process
the SIGINT signal, and call function '`save_log`' to save
downloaded blocks so that the download can be resumed.

`Resume.c (line 30)`

```C
void save_log()
{
  ...
  memcpy(&(h.wthread), wthread, sizeof(struct thread_data) * nthreads);
  ...
  h.bwritten = bwritten;
  ...
}
```

The '`memcpy`' function call saves the bytes downloaded by
each thread, and '`h.bwritten = bwritten`' saves the total
number of bytes downloaded by all thread. In normal
situation, the total number bytes downloaded by each thread
should be the same as '`bwritten`'. However, if the following
interleaving happens, the results will mismatch:

The buggy interleaving is like the following:

```C
Thread 1                                  Thread 2

void http_get(...)                        void save_log(...)
{                                         {
                                            ...
                                            memcpy(&(h.wthread), wthread, ...);
                                            ...
  td->offset += dw;

  pthread_mutex_lock(&bwritten_mutex);
  bwritten += dw;
  pthread_mutex_unlock(&bwritten_mutex);

                                            h.bwritten = bwritten;  
  ...                                       ...
}                                         }          
```