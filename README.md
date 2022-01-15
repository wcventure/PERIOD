*(PERIOD is a controlled concurrency testing tool, where a scheduler explores the space of possible interleavings of a concurrent program looking for bugs)*

# Search-Based Concurrency Interleavings Exploration

[![MIT License](https://img.shields.io/github/license/xiaocong/uiautomator.svg)](http://opensource.org/licenses/MIT)

This is a testing framework for concurency programs.

<div align=center>
<figure class="half">
    <img width=80% src="document/img/framework.png">
</figure>
</div>

----------

The following pages show the technique report:
  - [*Technical Report: Exploration-based Active Testing For Concurrency Bugs Detection.*](document/TechnicalReport.md)

The following pages show the implementation details of key techniques:
  - [*Static analysis to find key points that may cause data race/concurrency bugs.*](tool/staticAnalysis/README.md)
  - [*Perform Instrumentation based on static analysis.*](tool/staticAnalysis/LLVM-PASS/README.md)
  - [*Deadline Based Deterministic Scheduling (DBDS) to explore the key points interleavings.*](tool/DBDS/README.md)
  - [*Accelerate exploration and in-depth exploration.*](tool/DBDS/Strategy.md)

----------

The repository contains three folders: [*tool*](#tool), [*test*](#test) and [*evaluation*](#evaluation).

## Tool

We provide here a snapshot of ConFuzz. For simplicity, we provide shell script for the whole installation. Here, We recommend installing and running the tool under a Docker container. If you really want to install the tool in your developer environment, please see [INSTALL.md](tool/INSTALL.md)

#### Requirements

- Operating System: Ubuntu 18.04 LTS (*This is very important, as our implementation requires higher kernel version*)
- Run the following command to install Docker (*Docker version higher than 18.09.7*):
  ```sh
  $ sudo apt-get install docker.io
  ```
  (If you have any questions on docker, you can see [Docker's Documentation](https://docs.docker.com/install/linux/docker-ce/ubuntu/)).

#### Clone the Repository

```sh
$ git clone https://github.com/wcventure/ConcurrencyFuzzer.git ConFuzz --depth=1
$ cd ConFuzz
```

#### Build and Run the Docker Image

Firstly, system core dumps must be disabled as with AFL (you can skip this step sometimes. Keep going).

```sh
$ echo core|sudo tee /proc/sys/kernel/core_pattern
$ echo performance|sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

Run the following command to automatically build the docker image and configure the environment.

```sh
# build docker image
$ sudo docker build -t confuzzer --no-cache ./

# run docker image
$ sudo docker run --privileged -it confuzzer /bin/bash
```

If you want to build local souce, please read the instruction in Dockerfile.

## Test

Before you use UAFL fuzzer, we suggest that you first use those simple examples provided by us to confirm whether the tool can work normally. In the following, we use the examples in the test folder to explain how to use the tool.

#### Test with AddressSanitizer

[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) (aka ASan) is a memory error detector for C/C++. DBDS can be  performed with AddressSanitizer.

AddressSanitizer requires to add `-fsanitize=address` into CFLAGS or CXXFLAGS, we provide a llvm wapper. Take [`df.c`](test/doubleFree/df.c), which contains a simple double-free, as an example.

Before you start, you will have to have clang and llvm ready; use `pip3`
to have the `numpy` module install.
In addition, please download and install the C++ Boost library; therefore
you can compile and get the `dbds-clang-fast` instrumentation tool under
the `tool/staticAnalysis/DBDS-INSTRU` directory.

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/doubleFree/
$ ./cleanDIR.sh
$ clang -g -emit-llvm -c ./df.c -o df.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh df

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/test/doubleFree/ConConfig.df
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./df.c -o df.o
$ clang++ ./df.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o df -lpthread -fsanitize=address -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./df
```

#### Reproduce the Interleaving under a Certain Interleaving

After exectue `run_PDS.py`, it first perform dry run. Then we need to press Enter to continue. Finally, the results should look like following.

```sh
Start Testing!
test 0001: [0, 0, 1, 1]
test 0002: [0, 1, 0, 1]
        [Error Found]: NO.1 double-free
        The interleavings saved in out_df_1/Errors/000001
test 0003: [0, 1, 1, 0]
        [Error Found]: NO.2 double-free
        The interleavings saved in out_df_1/Errors/000002
test 0004: [1, 0, 0, 1]
        [Error Found]: NO.3 double-free
        The interleavings saved in out_df_1/Errors/000003
test 0005: [1, 0, 1, 0]
        [Error Found]: NO.4 double-free
        The interleavings saved in out_df_1/Errors/000004
test 0006: [1, 1, 0, 0]
End Testing!


Total Error Interleavings: 4
Total Timeouts Interleavings: 0
2 status found:
         [0, -6]
0 results found:
--------------------------------------------------
        Last New Find           Total
Round   0                       6
Time    00:00:00.00000          00:00:01.76925

```

From the result, we have found three interleavings that can lead to errors. The interleaving is saved in the folder `out_df_*`. If you want to reproduce a certain interleaving that saved in the folder `out_df_*`, you can perfrom the following command.

```bash
$ROOT_DIR/tool/DBDS/run_PDS.py -r out_df_1/Errors/000001 ./df
```

This command will execute the target program with interleaving `[0, 0, 1, 1]`. Actually, it can trigger a double-free bug.

```sh
=================================================================
==67534==ERROR: AddressSanitizer: attempting double-free on 0x602000000010 in thread T2:
    #0 0x4936fd  (/ConFuzz/test/doubleFree/df+0x4936fd)
    #1 0x4c5aa1  (/ConFuzz/test/doubleFree/df+0x4c5aa1)
    #2 0x7f586093e6b9  (/lib/x86_64-linux-gnu/libpthread.so.0+0x76b9)
    #3 0x7f585f9c74dc  (/lib/x86_64-linux-gnu/libc.so.6+0x1074dc)

0x602000000010 is located 0 bytes inside of 7-byte region [0x602000000010,0x602000000017)
freed by thread T1 here:
    #0 0x4936fd  (/ConFuzz/test/doubleFree/df+0x4936fd)
    #1 0x4c5a01  (/ConFuzz/test/doubleFree/df+0x4c5a01)

previously allocated by thread T0 here:
    #0 0x49397d  (/ConFuzz/test/doubleFree/df+0x49397d)
    #1 0x4c5baf  (/ConFuzz/test/doubleFree/df+0x4c5baf)
    #2 0x7f585f8e083f  (/lib/x86_64-linux-gnu/libc.so.6+0x2083f)

Thread T2 created by T0 here:
    #0 0x47e10a  (/ConFuzz/test/doubleFree/df+0x47e10a)
    #1 0x4c5c39  (/ConFuzz/test/doubleFree/df+0x4c5c39)
    #2 0x7f585f8e083f  (/lib/x86_64-linux-gnu/libc.so.6+0x2083f)
NULL 1

Thread T1 created by T0 here:
    #0 0x47e10a  (/ConFuzz/test/doubleFree/df+0x47e10a)
    #1 0x4c5c16  (/ConFuzz/test/doubleFree/df+0x4c5c16)
    #2 0x7f585f8e083f  (/lib/x86_64-linux-gnu/libc.so.6+0x2083f)

SUMMARY: AddressSanitizer: double-free (/ConFuzz/test/doubleFree/df+0x4936fd)
==67534==ABORTING
```

#### Test without AddressSanitizer

Before you use the tool, we suggest that you first use a simple example ([`increase_double.c`](test/increase_double/increase_double.c)) provided by us to determine whether the tool can work normally.

Please try to perform following command:

```bash
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

# perform PDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./increase_double
```

Then you will see that we find all ten different results.

#### Test with ThreadSanitizer

[ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) (aka TSan) is a fast data race detector for C/C++ and Go. DBDS can be performed with ThreadSanitizer.

ThreadSanitizer requires to add `-fsanitize=thread -fPIE -pie` into CFLAGS or CXXFLAGS, we provide a llvm wapper. Take [`increase_double.c`](test/increase_double/increase_double.c), which contains a simple double-free, as an example.

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/test/increase_double
$ ./cleanDIR.sh
$ clang++ -g -emit-llvm -c ./increase_double.cpp -o increase_double.bc

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh increase_double

# complie the instrumented program with ASAN
$ export Con_PATH=$ROOT_DIR/test/increase_double/ConConfig.increase_double
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=thread -fPIE -pie ./increase_double.cpp -o increase_double

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py ./increase_double
```

## Evaluation

The folder  evaluation contains all our evaluation subjects. After having the tool installed, you can run the script to build and instrument the subjects. After instrument the subjects you can run the script to perform testing on the subjects.

#### ConVul-CVE-Benchmarks

The benchmarks for paper "Detecting Concurrency Memory Corruption Vulnerabilities, ESEC/FSE 2019." are available in [this repository](https://github.com/mryancai/ConVul). It contains a set of concurrency vulnerabilities, including: UAF (Use After Free), NPD (Null Pointer Dereference), and DF (Double Free).

In `evaluation/ConVul-CVE-Benchmarks` folder, You can run the script `./build_all.sh` to automatically compile the programs in this benchmark:

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# complie all ConVul-CVE-Benchmarks programs 
$ cd $ROOT_DIR/evaluation/ConVul-CVE-Benchmarks
$ ./build_all.sh
```

For the test command for each CVE, refers to:
- [CVE-2009-3547](evaluation/ConVul-CVE-Benchmarks/CVE-2009-3547)
- [CVE-2011-2183](evaluation/ConVul-CVE-Benchmarks/CVE-2011-2183)
- [CVE-2013-1792](evaluation/ConVul-CVE-Benchmarks/CVE-2013-1792)
- [CVE-2015-7550](evaluation/ConVul-CVE-Benchmarks/CVE-2015-7550)
- [CVE-2016-1972](evaluation/ConVul-CVE-Benchmarks/CVE-2016-1972)
- [CVE-2016-1973](evaluation/ConVul-CVE-Benchmarks/CVE-2016-1973)
- [CVE-2016-7911](evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911)
- [CVE-2016-9806](evaluation/ConVul-CVE-Benchmarks/CVE-2016-9806)
- [CVE-2017-6346](evaluation/ConVul-CVE-Benchmarks/CVE-2017-6346)
- [CVE-2017-15265](evaluation/ConVul-CVE-Benchmarks/CVE-2017-15265)

**Result**: the Table shows the results of the all tools on 10 concurrency vulnerabilities. Our tool successfully detected all 10 vulnerabilites.

| **CVE ID** |  **Category** | **Program** | *Our Tool* | *ConVul* | *FT* | *HEL* | *TSAN* | *UFO* | *UFO<sub>NPD* |
|----------------|---------|------------------|-----|-----|-----|-----|-----|-----|-----|
| [CVE-2009-3547](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-3547)     | NDP     | Linux-2.6.32-rc6 | ✓ | ✓ | ✓ | ✓ | ✓ | - | ✓ |
| [CVE-2011-2183](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2011-2183)     | NDP     | Linux-2.6.39-3   | ✓ | ✗ | ✗ | ✗ | ✗ | - | ✗ |
| [CVE-2013-1792](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2013-1792)     | NDP     | Linux-2.8.3      | ✓ | ✓ | ✗ | ✗ | ✗ | - | ✗ |
| [CVE-2015-7550](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2015-7550)     | NDP     | Linux-4.3.4      | ✓ | ✓ | ✗ | ✗ | ✗ | - | ✓ |
| [CVE-2016-1972](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2016-1972)     | UAF     | Firefox-45.0     | ✓ | ✓ | ✗ | ✗ | ✗ | ✗ | - |
| [CVE-2016-1973](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2016-1973)     | UAF     | Firefox-45.0     | ✓ | ✓ | ✗ | ✗ | ✗ | ✗ | - |
| [CVE-2016-7911](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2016-7911)     | NDP     | Linux-4.6.6      | ✓ | ✓ | ✗ | ✗ | ✗ | - | ✗ |
| [CVE-2016-9806](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2016-9806)     | DF      | Linux-4.6.3      | ✓ | ✓ | ✗ | ✗ | ✗ | - | - |
| [CVE-2017-6346](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-6346)     | UAF(DF) | Linux-4.9.13     | ✓ | ✓ | ✗ | ✗ | ✗ | ✗ | - |
| [CVE-2017-15265](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-15265)   | UAF     | Linux-4.13.8     | ✓ | ✓ | ✗ | ✗ | ✓ | ✓ | - |
| **Total**                                                                         |         |                  | 10 | 9 | 1 | 1 | 2 | 1 | 2 |

**Remark**: In some CVE programs, DBDS also identified other types of bugs.
- CVE-2016-1972 program contains both UAF and NDP bug.
- CVE-2016-1973 program contains both UAF and NDP bug.
- CVE-2017-6346 program contains UAF, DF and NDP bug.


## Links

Website: https://sites.google.com/view/ConcurrencyFuzzer

GitHub: https://github.com/wcventure/ConcurrencyFuzzer
