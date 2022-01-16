# PERIOD: Controlled Concurrency Testing via Periodical Scheduling

[![MIT License](https://img.shields.io/github/license/xiaocong/uiautomator.svg)](http://opensource.org/licenses/MIT)

PERIOD is a controlled concurrency testing tool, where a scheduler explores the space of possible interleavings of a concurrent program looking for bugs.

This repository provides the tool and the evaluation subjects for the paper "Controlled Concurrency Testing via Periodical Scheduling'' accepted for the technical track at ICSE 2022. A preprint of the paper can be found at [ICSE2022_PERIOD.pdf](https://wcventure.github.io/pdf/ICSE2022_PERIOD.pdf).

**Table of contents**
- [Directory Structure](#directory-structure)
- [Tool](#tool)
  - [Requirements](#requirements)
  - [Installing](#installing)
  - [Running on Docker](#running-on-docker)
- [Test](#test)
- [Evaluation](#evaluation)
- [Advance Usage](#advance-usage)
- [Other Links](#other-links)
  - [Evaluation Dataset](#the-link-of-evaluation-dataset)
  - [Compared Baseline Tools](#the-link-of-compared-basedline-tools)
  
----------


## Directory Structure

The repository mainly contains three folders: [*tool*](#tool), [*test*](#test) and [*evaluation*](#evaluation). We briefly introduce each folder under `/workdir/PERIOD` (if you install by using docker, the root folder of the artifact would be `/workdir/PERIOD`):
- `clang+llvm`: Pre-Built Binaries of LLVM 10.0. These binaries include Clang, LLD, compiler-rt, various LLVM tools, etc.
- `tools`: Root directory for PERIOD tool and scripts.
  - `DBDS`: The code about our proposed periodical scheduling method that can systematically explore the thread interleaving.
  - `staticanalysis`: The scripts of static analysis, which aim to find key points that used to be scheduled during testing. It also contains the codes about instrumentation.
  - `SVF`: Third-party libraries project SVF, which provides interprocedural dependence analysis for LLVM-based languages. SVF is able to perform pointer alias analysis, memory SSA form construction, value-flow tracking for program variables. 
  - `wllvm`: Third-party libraries project WLLVM, which provides tools for building whole-program (or whole-library) LLVM bitcode files from an unmodified C or C++ source package.
- `test`: Some simple examples that are easy to understand. These examples could also be used to check the installation.
- `evaluation`: Root for benchmark directories. The ConVul-CVE-Benchmarks correspond to the 10 programs used in Table. 1 in our paper. Other folders contain the 36 programs used in Table. 2 in our paper. We provide scripts for each program for easy installation.
  - `ConVul-CVE-Benchmarks`: 10 programs correspond to a real-world CVE.
  - `CB`: Test cases for real applications.
  - `CS`: Small test cases and some small programs.
  - `Chess`: Test cases for several versions of a work-stealing queue.
  - `Inspect_benchmarks`: Small test cases and some small programs.
  - `RADBench`: Tests cases for real applications.
  - `SafeStack`: Test case for lock-free stack and a debugging library test case.
  - `Splash2`: Parallel workloads.

----------

## Tool

The easiest way to use PERIOD is to use Docker. We strongly recommend installing and running our tool based on Docker. If you really want to install the tool in your host system, please see [INSTALL.md](tool/INSTALL.md) for more detail.

### Requirements

- **Hardware Requirements:** Please use workstations/PC with multi-core processors, as PERIOD is a concurrency testing tool
- **Operating System:** >= Ubuntu 18.04 LTS (Requires kernel version >= 4.x due to the scheduling policy)
- **Docker**: The only requirement is to install Docker (version higher than 18.09.7). You can use the command `sudo apt-get install docker.io` to install the docker on your Linux machine. (If you have any questions on docker, you can see [Docker's Documentation](https://docs.docker.com/engine/install/ubuntu/)).

### Installing

- **Add `period:latest` image on your system**. There are two ways of doing this:
  - `docker pull wcventure/period:stable && docker tag wcventure/period:stable period:latest`
  - Alternatively, you can build your own image with `sudo docker build -t period:latest --no-cache ./`

### Running on Docker

- **Running a privileged container**
  - `sudo docker run --privileged -it period:latest /bin/bash`
  - Then you will start a docker container and enter the configured environment of PERIOD.

----------

## Test

Before you use PERIOD, we suggest that you first use those simple examples provided by us to confirm whether the tool can work normally. In the following, we use the examples in the `test` folder to explain how to use the tool.

You can use one of the programs in the `test` folder to check whether PERIOD works normally. Take [`doubleFree`](test/doubleFree/df.c) as a running example. Here, let's run it with a prepared script (If you want to try a program outside the folder `test` and `evaluation`, please refer to [Advance Usage](AdvancUsage.md).).

1. Enter the working directory:
    ```sh
    cd /workdir/PERIOD/test/doubleFree
    ```

2. Use the following script to build the program:
   ```sh
   ./cleanDIR.sh && ./build.sh
   ```

3. Perform systematic controlled concurrency testing based on periodical scheduling:
    ```sh
    $ROOT_DIR/tool/DBDS/run_PDS.py -y -d 3 ./df
    ```

4. If PERIOD works normally, you can see the following outputs, which indicate that PERIOD reports 2 buggy interleavings in this program (i.e., double free bugs):
    ```sh
    Start Testing!
    Targeting bugs that bug depth = 1 . Iterate for 2 periods
    test 0001: [[0, 0], [1, 1]]
    test 0002: [[1, 1], [0, 0]]
    Targeting bugs that bug depth = 2 . Iterate for 3 periods
    test 0003: [[0], [1, 1], [0]]
            [Error Found]: NO.1 double-free
            The interleavings saved in out_df_1/Errors/000001_0000003_double-free
    test 0004: [[1], [0, 0], [1]]
            [Error Found]: NO.2 double-free
            The interleavings saved in out_df_1/Errors/000002_0000004_double-free
    Targeting bugs that bug depth = 3 . Iterate for 4 periods
    test 0005: [[0], [1], [0], [1]]
            [Error Found]: NO.3 double-free
            The interleavings saved in out_df_1/Errors/000003_0000005_double-free
    test 0006: [[1], [0], [1], [0]]
            [Error Found]: NO.4 double-free
            The interleavings saved in out_df_1/Errors/000004_0000006_double-free
    End Testing!
    ```

5. After the testing terminates, you can find the folder out_work_1, which saves the buggy schedule that could deterministically reproduce the bugs. 
    - Use the command like the following to reproduce the double free:
    ```sh
    $ROOT_DIR/tool/DBDS/run_PDS.py -r out_df_1/Errors/000001_0000003_double-free ./df
    ```
    And you will see the double free reported by ASAN:
    ```sh
    ==6742==ERROR: AddressSanitizer: attempting double-free on 0x602000000010 in thread T2:
    NULL 1
        #0 0x4941dd  (/workdir/PERIOD/test/doubleFree/df+0x4941dd)
        #1 0x4c65b1  (/workdir/PERIOD/test/doubleFree/df+0x4c65b1)
        #2 0x7f9f056bc6da  (/lib/x86_64-linux-gnu/libpthread.so.0+0x76da)
        #3 0x7f9f0469a71e  (/lib/x86_64-linux-gnu/libc.so.6+0x12171e)
    ...
    SUMMARY: AddressSanitizer: double-free (/workdir/PERIOD/test/doubleFree/df+0x4941dd)
    ==6742==ABORTING
    ```

6. If the above steps can be executed normally, it means that your installation has been successful！You can continue to try other examples in the `test` folder. There are a list of simple examples that are easy to understand in the `test` folder:
   - [UAF](test/UAF)
   - [doubleFree](test/doubleFree)
   - [doubleFree2](test/doubleFree2)
   - [increase_double](test/increase_double)
   - [issue205](test/issue205)
   - [lock](test/lock)
   - [null_concurrency_race](test/null_concurrency_race)
   - [test](test/test)
   - [transfer](test/transfer)
   - [work](test/work)


----------


## Evaluation

The fold evaluation contains all our evaluation subjects. After having PERIOD installed, you can run the scripts `./build.sh` to automatically build the benchmark program with PERIOD instrumentation. Then run the script `$ROOT_DIR/tool/DBDS/run_PDS.py` to perform systematic controlled concurrency testing based on periodical scheduling, with the command `$ROOT_DIR/tool/DBDS/run_PDS.py -d <periods limits> -t <timeout> <program under test> <args>`. 

We take the `CVE-2016-7911` in CVE-Benchmark as an example here.
1. Build Benchmark Program
```sh
cd /workdir/PERIOD/evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911
./cleanDIR.sh
./build.sh
```

2. Expose Bugs with PERIOD
```sh
$ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./2016-7911
```

3. You may see the following output, which means that we found 8 buggy interleavings related to SEGV (the 3rd tested interleaving found the first SEGV bug).
```sh
Start Testing!
Targeting bugs that bug depth = 1 . Iterate for 2 periods
test 0001: [[0, 0, 0], [1, 1, 1, 1]]
test 0002: [[1, 1, 1, 1], [0, 0, 0]]
Targeting bugs that bug depth = 2 . Iterate for 3 periods
test 0003: [[0], [1, 1, 1, 1], [0, 0]]
        [Error Found]: NO.1 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000001_0000003_SEGV
test 0004: [[0, 0], [1, 1, 1, 1], [0]]
        [Error Found]: NO.2 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000002_0000004_SEGV
test 0005: [[1], [0, 0, 0], [1, 1, 1]]
test 0006: [[1, 1], [0, 0, 0], [1, 1]]
test 0007: [[1, 1, 1], [0, 0, 0], [1]]
Targeting bugs that bug depth = 3 . Iterate for 4 periods
test 0008: [[0], [1], [0, 0], [1, 1, 1]]
test 0009: [[0], [1, 1], [0, 0], [1, 1]]
test 0010: [[0], [1, 1, 1], [0, 0], [1]]
        [Error Found]: NO.3 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000003_0000010_SEGV
test 0011: [[0, 0], [1], [0], [1, 1, 1]]
test 0012: [[0, 0], [1, 1], [0], [1, 1]]
test 0013: [[0, 0], [1, 1, 1], [0], [1]]
        [Error Found]: NO.4 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000004_0000013_SEGV
test 0014: [[1], [0], [1, 1, 1], [0, 0]]
        [Error Found]: NO.5 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000005_0000014_SEGV
test 0015: [[1, 1], [0], [1, 1], [0, 0]]
        [Error Found]: NO.6 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000006_0000015_SEGV
test 0016: [[1, 1, 1], [0], [1], [0, 0]]
test 0017: [[1], [0, 0], [1, 1, 1], [0]]
        [Error Found]: NO.7 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000007_0000017_SEGV
test 0018: [[1, 1], [0, 0], [1, 1], [0]]
        [Error Found]: NO.8 SEGV
        The interleavings saved in out_2016-7911_1/Errors/000008_0000018_SEGV
test 0019: [[1, 1, 1], [0, 0], [1], [0]]
End Testing!


Total Error Interleavings: 8
Total Timeouts Interleavings: 0
```

4. Deterministically Reproduce the Bug
```sh
$ROOT_DIR/tool/DBDS/run_PDS.py -r out_2016-7911_1/Errors/000001_0000003_SEGV ./2016-7911
```
You can see a SEVG dump by ASAN:
```sh
==1592==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000004 (pc 0x0000004de9cf bp 0x000000000000 sp 0x7fab08670e50 T1)
==1592==The signal is caused by a READ memory access.
==1592==Hint: address points to the zero page.
    #0 0x4de9cf  (/workdir/PERIOD/evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911/2016-7911+0x4de9cf)
    #1 0x4de90f  (/workdir/PERIOD/evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911/2016-7911+0x4de90f)
    #2 0x7fab0c3076da  (/lib/x86_64-linux-gnu/libpthread.so.0+0x76da)
    #3 0x7fab0b2e571e  (/lib/x86_64-linux-gnu/libc.so.6+0x12171e)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV (/workdir/PERIOD/evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911/2016-7911+0x4de9cf)
Thread T1 created by T0 here:
    #0 0x48c76a  (/workdir/PERIOD/evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911/2016-7911+0x48c76a)
    #1 0x4debc8  (/workdir/PERIOD/evaluation/ConVul-CVE-Benchmarks/CVE-2016-7911/2016-7911+0x4debc8)
    #2 0x7fab0b1e5bf6  (/lib/x86_64-linux-gnu/libc.so.6+0x21bf6)

==1592==ABORTING
```

#### CVE Benchmarks

The benchmarks for paper "Detecting Concurrency Memory Corruption Vulnerabilities, ESEC/FSE 2019." are available in [this repository](https://github.com/mryancai/ConVul). It contains a set of concurrency vulnerabilities, including: UAF (Use After Free), NPD (Null Pointer Dereference), and DF (Double Free).

In `evaluation/ConVul-CVE-Benchmarks` folder, You can run the script `./build_all.sh` to automatically compile the programs in this benchmark:

```bash
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile all ConVul-CVE-Benchmarks programs 
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


#### SCTBench

The benchmarks for paper "Concurrency testing using controlled schedulers: An empirical study, TOPC 2016." are available in [this repository](https://github.com/mc-imperial/sctbench). SCTBench collects concurrency bugs from previous parallel workloads (SPLASH2) and concurrency testing/verification works (CB , CHESS, CS and Inspect). For the test command for each benchmark programs, refers to:
- [CB](evaluation/CB)
- [CS](evaluation/CS)
- [Chess](evaluation/Chess)
- [Inspect](evaluation/Inspect_benchmarks)
- [RADBench](evaluation/RADBench)
- [SafeStack](evaluation/SafeStack)
- [Splash2](evaluation/Splash2)

----------

## Advance Usage

If you want to try a program outside the folder `test` and `evaluation`, please refer to [Advance Usage](AdvancUsage.md).

----------


## Publications

Note that the DATA and CODE are free for Research and Education Use ONLY. 
Please cite our paper (add the bibtex below) if you use any part of our ALGORITHM, CODE, DATA or RESULTS in any publication.

```
@inproceedings{wen2022period,
author = {Wen, Cheng and He, Mengda and Wu, Bohao and Xu, Zhiwu and Qin, Shengchao},
title = {Controlled Concurrency Testing via Periodical Scheduling},
year = {2022},
isbn = {9781450392211},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
url = {https://doi.org/10.1145/3510003.3510178},
doi = {10.1145/3510003.3510178},
booktitle = {Proceedings of the ACM/IEEE 4th International Conference on Software Engineering},
keywords = {software vulnerability, memory consumption, fuzz testing},
location = {Pittsburgh, USA},
series = {ICSE '22}
}

```

----------


## Other Links

### The Link of Evaluation Dataset

We have included all the benchmark programs used in our paper into the docker image, with our scripts for easy configuration. If you need the original dataset, it could be download from:
- STCBench: [https://github.com/mc-imperial/sctbench](https://github.com/mc-imperial/sctbench)
- CVE benchmark: [https://github.com/mryancai/ConVul](https://github.com/mryancai/ConVul)

### The Link of Compared Baseline Tools

We used existing implementations of compared tools when available. Here are their websites to obtain the tool:
- Maple: https://github.com/jieyu/maple.
- IPB, IDB, DFS, PCT, Random are optimized and implemented in https://github.com/mc-imperial/sctbench.
- ConVul can be downloaded on https://sites.google.com/site/detectconvul/.
- UFO can be downloaded on https://github.com/parasol-aser/UFO.
- Data Race detector: [TSAN](https://github.com/google/sanitizers), [FastTrack](https://github.com/microsoft/FastTrack), [Helgrind+](https://valgrind.org/docs/manual/hg-manual.html).

