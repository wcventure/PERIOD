# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# download the source code
$ cd $ROOT_DIR/evaluation/RADBench/bug5/src
$ wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8.6/src/nspr-4.8.6.tar.gz

# compile the program, perform static analysis and complie the instrumented program with ASAN
$ cd $ROOT_DIR/evaluation/RADBench/bug5
$ ./cleanDIR.sh
$ make

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 LD_LIBRARY_PATH=$ROOT_DIR/evaluation/RADBench/bug5/src/nspr-4.8.6/target/dist/lib/ bin/main
```

Then you will get the results.

```sh
Start Testing!

...
```

We can find some schedules that can cause Deadlock.

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
Bug 5 (Bug #165586)
a.k.a NSPR-2
------------------------------

------------------------------
Description
------------------------------

NSPR Bug

https://bugzilla.mozilla.org/show_bug.cgi?id=165586

(or docs folder) for more information.

------------------------------
Relevant Functions
------------------------------

`PR_WaitCondVar (ptsynch.c)`
`PR_Interrupt (ptthread.c)`

------------------------------
Makefile Targets
------------------------------

The Makefile has three targets

- make: This does the default buggy build
- make sleep: this does the default buggy build but with an extra sleep added in
ptsynch.c and ptthread.c to make the bug more likely to occur

------------------------------
Where is the source from?
------------------------------

The source for nspr was retrieved by executing the following commands

```sh
$ wget http://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8.6/src/nspr-4.8.6.tar.gz
```

------------------------------
Bug
------------------------------

An atomicity violation in the condition variable wait can result in *deadlock*.

Atomicity violation should occur at `/mozilla/nsprpbu/pr/src/pthreads/ptsynch.c` at line 399.
Also check out `/mozilla/nsprpbu/pr/src/pthreads/ptthread.c` at line 750.

