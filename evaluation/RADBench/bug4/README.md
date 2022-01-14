# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# download the source code
$ cd $ROOT_DIR/evaluation/RADBench/bug4/src
$ wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.4/src/nspr-4.6.4.tar.gz

# compile the program, perform static analysis and complie the instrumented program
$ cd $ROOT_DIR/evaluation/RADBench/bug4
$ ./cleanDIR.sh
$ make

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 bin/main
```

Then you will get the results.

```sh

Start Testing!
test 0001: [0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1]
test 0002: [0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1]
test 0003: [0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1]
test 0004: [0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1]
test 0005: [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1]
test 0006: [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1]
test 0007: [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0]
test 0008: [0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1]
test 0009: [0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1]
test 0010: [0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1]
...
test 0460: [0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0]
test 0461: [0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0]
test 0462: [0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0]
test 0463: [0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1]
test 0464: [0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1]
        [Error Found]: NO.1
        The interleavings saved in out_main_1/Errors/000001
test 0465: [0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1]
        [Error Found]: NO.2
        The interleavings saved in out_main_1/Errors/000002
...

```

We can find some schedulesthat can cause Assertion failure.

```sh
Assertion failure: pthread_equal(lock->owner, pthread_self()), at ../../../../mozilla/nsprpub/pr/src/pthreads/ptsynch.c:225
Assertion failure: _PT_PTHREAD_MUTEX_IS_LOCKED(lock->mutex), at ../../../../mozilla/nsprpub/pr/src/pthreads/ptsynch.c:223
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
Bug 4 (Bug #354593)
a.k.a NSPR-1
------------------------------

------------------------------
Description
------------------------------

NSPR Bug

https://bugzilla.mozilla.org/show_bug.cgi?id=354593

(or docs folder) for more information.

------------------------------
Relevant Functions
------------------------------

MT_safe_localtime (prtime.c)

------------------------------
Makefile Targets
------------------------------

The Makefile has three targets

make:
This does the default buggy build

make sleep: 
this does the default buggy build but with an extra sleep added in
prtime.c to make the bug more likely to occur

make patch: 
this applies the bug fix and then does the build

------------------------------
Where is the source from?
------------------------------

The source for nspr was retrieved by executing the following commands

```
$ wget http://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.4/src/nspr-4.6.4.tar.gz
```

------------------------------
Bug
------------------------------

A poorly synchronized initialization can result in multiple copies of a
specific monitor mutex being allocated.  This can allow multiple threads into a
single critical section.  Manifests as an assertion presumably because the NSPR
library does some tracking in critical sections to make sure everything is
sane.

See mozilla/nsprpub/pr/src/misc/prtime.c at line ~580.

[najalber@najalber-c5 ~/bugs/bug4/src/nspr-4.6.4/target]$ LD_LIBRARY_PATH=/home/najalber/bugs/bug4/src/nspr-4.6.4/target/dist/lib /home/najalber/bugs/bug4/bin/test-time
Assertion failure: pthread_equal(lock->owner, pthread_self()), at ../../../../mozilla/nsprpub/pr/src/pthreads/ptsynch.c:225
Aborted

