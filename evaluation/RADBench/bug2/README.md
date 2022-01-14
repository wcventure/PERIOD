# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# download the source code
$ cd $ROOT_DIR/evaluation/RADBench/bug2/src
$ wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.7/src/nspr-4.6.7.tar.gz
$ wget https://ftp.mozilla.org/pub/mozilla.org/js/js-1.7.0.tar.gz

# compile the program, perform static analysis and complie the instrumented program with ASAN
$ cd $ROOT_DIR/evaluation/RADBench/bug2
$ ./cleanDIR.sh
$ make

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 3 LD_LIBRARY_PATH=$ROOT_DIR/evaluation/RADBench/bug2/src/js/src/Linux_All_DBG.OBJ:$ROOT_DIR/evaluation/RADBench/bug2/src/nspr-4.6.7/target/dist/lib/ LOOP_UNWINDED_NUM=1 $ROOT_DIR/evaluation/RADBench/bug2/bin/main
```

Then you will get the results.

```sh
Start Testing!
test 0001: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0002: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0003: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0004: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0005: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0006: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0007: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0008: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1]
test 0009: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1]
test 0010: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1]
test 0011: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1]
test 0012: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1]
test 0013: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1]
	[Error Found]: NO.1 SEGV
	The interleavings saved in out__2/Errors/000001 
test 0014: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1]
...
```

We can find some schedules that can cause NULL-Pointer-Deference. The ASAN output are shown as follow:

```sh
AddressSanitizer:DEADLYSIGNAL
=================================================================
==14939==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000018 (pc 0x7f478985a1fc bp 0x7f4785f23c40 sp 0x7f4785f233f8 T2)
==14939==The signal is caused by a WRITE memory access.
==14939==Hint: address points to the zero page.
    #0 0x7f478985a1fc  (/lib/x86_64-linux-gnu/libc.so.6+0x18f1fc)
    #1 0x43ee50  (/ConcurrencyFuzzer/evaluation/RADBench/bug2/bin/main+0x43ee50)
    #2 0x7f478a881529  (/ConcurrencyFuzzer/evaluation/RADBench/bug2/src/js/src/Linux_All_DBG.OBJ/libjs.so+0x7a529)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV (/lib/x86_64-linux-gnu/libc.so.6+0x18f1fc) 
Thread T2 created by T0 here:
    #0 0x48bbaa  (/ConcurrencyFuzzer/evaluation/RADBench/bug2/bin/main+0x48bbaa)
    #1 0x4dc368  (/ConcurrencyFuzzer/evaluation/RADBench/bug2/bin/main+0x4dc368)
    #2 0x7f47896ecb96  (/lib/x86_64-linux-gnu/libc.so.6+0x21b96)

==14939==ABORTING
```

Use `addr2line -e src/js/src/Linux_All_DBG.OBJ/libjs.so 0x7a529` to see the debug info
```
/ConcurrencyFuzzer/evaluation/RADBench/bug2/src/js/src/jsgc.c:2846
```

# Notes

```
 "INTEL CONFIDENTIAL
 Copyright (February 2011) Intel Corporation All Rights Reserved.
 
 The source code contained or described herein and all documents
 related to the source code ("Material") are owned by Intel Corporation
 or its suppliers or licensors. Title to the Material remains with
 Intel Corporation or its suppliers and licensors. The Material may
 contain trade secrets and proprietary and confidential information of
 Intel Corporation and its suppliers and licensors, and is protected by
 worldwide copyright and trade secret laws and treaty provisions. No
 part of the Material may be used, copied, reproduced, modified,
 published, uploaded, posted, transmitted, distributed, or disclosed in
 any way without Intel~Rs prior express written permission.  No
 license under any patent, copyright, trade secret or other
 intellectual property right is granted to or conferred upon you by
 disclosure or delivery of the Materials, either expressly, by
 implication, inducement, estoppel or otherwise. Any license under such
 intellectual property rights must be express and approved by Intel in
 writing.
 
 Include any supplier copyright notices as supplier requires Intel to use.
 
 Include supplier trademarks or logos as supplier requires Intel to
 use, preceded by an asterisk. An asterisked footnote can be added as
 follows: *Third Party trademarks are the property of their respective
 owners.
 
 Unless otherwise agreed by Intel in writing, you may not remove or
 alter this notice or any other notice embedded in Materials by Intel
 or Intel~Rs suppliers or licensors in any way."
```

------------------------------
Bug 2 (Bug #476934)
a.k.a. SpiderMonkey-1
------------------------------

------------------------------
Description
------------------------------

Firefox Javascript Bug--atomicity violation in garbage collector can result in
null pointer dereference. See docs folder or

https://bugzilla.mozilla.org/show_bug.cgi?id=476934

for more information.

------------------------------
Relevant Functions
------------------------------


`JS_SetContextThread (jsapi.cpp)`
`{JS,js}_ClearContextThread (jsapi.cpp, jscntxt.cpp)`
`JS_BeginRequest (jsapi.cpp)`
`{JS,js}_GC (jsapi.cpp, jsgc.cpp)`
`JS_EndRequest (jsapi.cpp)`

------------------------------
Where is the source from?
------------------------------

The source comes from the separately released NSPR 4.6.7 (July 6, 2007)
threading package and JS 1.7.0 engine (Oct 19, 2007):

```sh
$ wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.7/src/nspr-4.6.7.tar.gz
$ wget ftp://ftp.mozilla.org/pub/mozilla.org/js/js-1.7.0.tar.gz
```
------------------------------
Scripted Build
------------------------------

in the scripts/ folder contains:

- build-js.sh: automates the buggy source build.
- build-testharness.sh: build script for the test harness.
- clean.sh: deletes test harness, unzipped sources, and built binaries.

------------------------------
The Bug
------------------------------
```sh
$ LD_LIBRARY_PATH=src/js/src/Linux_All_DBG.OBJ bin/test-ctxt
Segmentation fault
```

This bug manifests fairly rarely.  To make it occur more often try increasing
THREADS in src/test-ctxt.cpp and recompiling the test
harness.

There is some debate in the bug report about whether this bug is a misuse of
the JS engine API.  Comment 4 by Brendan Eich suggests to me that this is
indeed a valid way to use the API. Specifically:

"Igor, it was never legal to use JS_THREADSAFE but not JS_BeginRequest(cx)
before calling JS_DestroyContext*.

JS_ClearContextThread is a different issue, arguably, although it too could be
called in a request (on a different context, say a root or dummy context)."

That is, you cannot call JS_DestroyContext*() outside of a request but
JS_ClearContext() could arguably be called outside of a request.  I note this
because it seems making the JS_ClearContext() call inside a request seems to
prevent the segfault.




