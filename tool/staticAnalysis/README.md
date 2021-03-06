# Static Analysis

This page shows how to perform our static analysis to find key points that may cause potential data race/concurrency bugs.

-----

## Quick Start

We use the file [transfer.c](../../test/UAF/uaf.c) as our running example.

Before performing static analysis for `uaf.c`, we need to compile the program and obtain its bitcode file:

```bash
$ export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
$ export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
$ cd $ROOT_DIR/test/UAF
$ clang -g -O -emit-llvm -c ./uaf.c -o uaf.bc
```

Then use the script `staticAnalysis.sh` static to perform the static analysis:

```bash
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh uaf
```

Then you can see the memGroups in the file `mempair.uaf.npz`.

```sh
# Var, Write, Read, SensitiveOperations
[12, set(), {'uaf.c:29', 'uaf.c:14', 'uaf.c:27', 'uaf.c:28', 'uaf.c:19', 'uaf.c:16', 'uaf.c:18'}, set()]
[15, {'uaf.c:15'}, set(), set()]
[18, set(), {'uaf.c:17'}, {'uaf.c:20'}]
[20, {'uaf.c:17'}, {'uaf.c:30'}, set()]
[116, {'uaf.c:30'}, set(), set()]
```

And you can see the mempairs in the file `mempair.uaf`.

```sh
uaf.c:15 uaf.c:15 W W
uaf.c:17 uaf.c:17 W W
uaf.c:17 uaf.c:20 R free
uaf.c:17 uaf.c:30 W R
uaf.c:20 uaf.c:20 free free
uaf.c:30 uaf.c:30 W W
```


-----


## Detail Workflow

This Section is too detailed. For ordinary users, using using the top-level  the script `staticAnalysis.sh` is enough. 

The script `staticAnalysis.sh` consists of several parts.


### Use SVF to Generate a Trimmed Memory SSA.

SVF is a static tool that enables scalable and precise interprocedural dependence analysis for C and C++ programs. SVF allows value-flow construction and pointer analysis to be performed iteratively, thereby providing increasingly improved precision for both. SVF accepts the points-to information generated by any pointer analysis (e.g., Andersen’s analysis) and constructs an interprocedural [memory SSA](https://llvm.org/docs/MemorySSA.html) form so that the def-use chains of both top-level and address-taken variables are captured. SVF is implemented on top of an industry-strength compiler [LLVM](http://llvm.org/).

We use a modified version of SVF to dump SVFG and Trimmed Memory SSA, by using following command:

```bash
$ export PATH=$ROOT_DIR/tool/SVF/Release-build/bin/:$PATH
$ $ROOT_DIR/tool/staticAnalysis/analysis.py ./uaf.bc > ./mssa.uaf
```

We can get a trimmed memory SSA for function racy_transfer, it looks like the following outputs:

```sh
==========FUNCTION: racy_transfer==========

==========FUNCTION: t1==========

LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:14:5]]({i8})
9V_2 = STCHI(MR_9V_1) 	pts{15 }
	[[./uaf.c:15:7]]({i32})


LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:16:5]]({i8})

LDMU(MR_11V_1) 	pts{18 }
	[[./uaf.c:17:10]]({i64})
4V_2 = STCHI(MR_4V_1) 	pts{20 }
	[[./uaf.c:17:8]]({i64})


LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:18:5]]({i8})

LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:19:6]]({i8})

LDMU(MR_11V_1) 	pts{18 }
	[[./uaf.c:20:9]]({i8*})
==========FUNCTION: free==========
==========FUNCTION: t2==========

LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:27:5]]({i8})

LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:28:6]]({i8})

LDMU(MR_2V_1) 	pts{12 }
	[[./uaf.c:29:7]]({i8})

LDMU(MR_4V_1) 	pts{20 }
	[[./uaf.c:30:5]]({i8*})
6V_2 = STCHI(MR_6V_1) 	pts{116 }
	[[./uaf.c:30:13]]({i8})

==========FUNCTION: main==========
9V_2 = STCHI(MR_9V_1) 	pts{15 }
	[[./uaf.c:37:6]]({i32})

11V_2 = STCHI(MR_11V_1) 	pts{18 }
	[[./uaf.c:38:7]]({i8*})

4V_2 = STCHI(MR_4V_1) 	pts{20 }
	[[./uaf.c:39:7]]({i8*})

2V_2 = STCHI(MR_2V_1) 	pts{12 }
	[[./uaf.c:42:8]]({i8})

2V_3 = STCHI(MR_2V_2) 	pts{12 }
	[[./uaf.c:43:8]]({i8})

2V_4 = STCHI(MR_2V_3) 	pts{12 }
	[[./uaf.c:44:8]]({i8})

2V_5 = STCHI(MR_2V_4) 	pts{12 }
	[[./uaf.c:45:8]]({i8})

2V_6 = STCHI(MR_2V_5) 	pts{12 }
	[[./uaf.c:46:8]]({i8})

2V_7 = STCHI(MR_2V_6) 	pts{12 }
	[[./uaf.c:47:8]]({i8})

2V_8 = STCHI(MR_2V_7) 	pts{12 }
	[[./uaf.c:48:8]]({i8})


LDMU(MR_9V_3) 	pts{15 }
	[[./uaf.c:56:9]]({i32})
```


### From Memory SSA to MemGroup

We then extract the aliased pair from memory SSA. We provide `get_aliased_info.py` to obtain aliased group.

```bash
${ROOT_DIR}/tool/staticAnalysis/get_aliased_info.py ./mssa.uaf
```

We get the aliased group, as shown in the following. For example, `[15, {'uaf.c:37', 'uaf.c:15'}, {'uaf.c:56'}]` means that there is a memory read for variable `15` at `uaf.c:56`, and there is also a memory write for variable `15` at `uaf.c:37` and `uaf.c:15`.

```sh
[[12, {'uaf.c:46', 'uaf.c:42', 'uaf.c:44', 'uaf.c:47', 'uaf.c:45', 'uaf.c:48', 'uaf.c:43'}, {'uaf.c:27', 'uaf.c:16', 'uaf.c:19', 'uaf.c:28', 'uaf.c:29', 'uaf.c:18', 'uaf.c:14'}]
[15, {'uaf.c:37', 'uaf.c:15'}, {'uaf.c:56'}]
[18, {'uaf.c:38'}, {'uaf.c:17', 'uaf.c:20'}]
[20, {'uaf.c:17', 'uaf.c:39'}, {'uaf.c:30'}]
[116, {'uaf.c:30'}, set()]]
```


### Get single Line for Instrumentation

When performing instrumentation, we only focus on the line number.

```bash
${ROOT_DIR}/tool/staticAnalysis/get_aliased_info.py ./mssa.uaf > ./ConConfig.uaf
```

The results are looks like the following output:

```sh
uaf.c:14
uaf.c:15
uaf.c:16
uaf.c:17
uaf.c:18
uaf.c:19
uaf.c:20
uaf.c:27
uaf.c:28
uaf.c:29
uaf.c:30
uaf.c:37
uaf.c:38
uaf.c:39
uaf.c:42
uaf.c:43
uaf.c:44
uaf.c:45
uaf.c:46
uaf.c:47
uaf.c:48
uaf.c:56
```


### Analyze Sensitive Memory Operations

We use a LLVM PASS to analyze sensitive memory operations, such as free(x), x=NULL, etc. We perform some static analysis in this LLVM Pass, too.

```bash
$ export Con_PATH=${ROOT_DIR}/test/UAF/ConConfig.uaf
$ opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/SensitiveOperationsPass/libSensitiveOperationsPass.so -so ./uaf.bc -o /dev/null > so.uaf
```

We can get the sensitive memory operations, as shown in the following:

```sh
uaf.c:20 free t1 19819499416024848
```

Then, use to `updateMemGroupSo.py` to update memGroup. This considers the information of thread analysis and call graph analysis. Thus we get trimmed memGroup.

```bash
$ROOT_DIR/tool/staticAnalysis/updateMemGroupSo.py -i ./so.uaf -m ./memGroup.npy > ./ConConfig.uaf
```

The memGroup could be:

```sh
# Var, Write, Read, SensitiveOperations
[[15, {'uaf.c:15'}, set(), set()], 
[18, set(), {'uaf.c:17'}, {'uaf.c:20'}],
[20, {'uaf.c:17'}, {'uaf.c:30'}, set()], 
[116, {'uaf.c:30'}, set(), set()]]
```

The `ConConfig.uaf` is also be updated.

```sh
uaf.c:15
uaf.c:17
uaf.c:20
uaf.c:30
```


### From MemGroup to MemPair

We then extract the aliased pair from memory group. We provide `DumpMempair.py` to obtain aliased_pair.

```bash
$ $ROOT_DIR/tool/staticAnalysis/DumpMempair.py -o mempair.uaf.npz
```

We get the aliased pairs, as shown in the following. For example, `uaf.c:17 uaf.c:20 R free` means that there is a memory read for a certain variable at transfer.c:17, and there is also a memory `free()` for a certain variable at `transfer.c:20`.

```sh
uaf.c:15 uaf.c:15 W W
uaf.c:17 uaf.c:17 W W
uaf.c:17 uaf.c:20 R free
uaf.c:17 uaf.c:30 W R
uaf.c:20 uaf.c:20 free free
uaf.c:30 uaf.c:30 W W
```

The memory group, memory pairs, as well as so_info are saved in `mempair.uaf.npz`.