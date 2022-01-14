# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# download the source code
$ cd $ROOT_DIR/evaluation/RADBench/bug6/src
$ wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8/src/nspr-4.8.tar.gz

# compile the program, perform static analysis and complie the instrumented program
$ cd $ROOT_DIR/evaluation/RADBench/bug6
$ ./cleanDIR.sh
$ make

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 2 bin/main
```

Then you will get the results.

```sh
Start Testing!
test 0001: [0, 0, 1]
test 0002: [0, 1, 0]
        [Timeout 3]: NO.1
        under interleavings saved in out_main_1/Timeouts/000001
test 0003: [1, 0, 0]
End Testing!
```

There is a schedule (i.e., [0, 1, 0]) that can cause a deadlock.


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
Bug 6 #526805)
a.k.a. NSPR-3
------------------------------

------------------------------
Description
------------------------------

NSPR Bug

https://bugzilla.mozilla.org/show_bug.cgi?id=526805

(or docs folder) for more information.

------------------------------
Relevant Functions
------------------------------

`PR_RWLock_Wlock` (prrwlock.c)
`PR_RWLock_Rlock` (prrwlock.c)
`PR_RWLock_Unlock` (prrwlock.c)

------------------------------
Makefile Targets
------------------------------

The Makefile has two targets

- make: This does the default buggy build

- make sleep: this does the default buggy build but with an extra sleep added in
ptsynch.c and ptthread.c to make the bug more likely to occur

------------------------------
Where is the source from?
------------------------------

The source for nspr was retrieved by executing the following commands

```
$ wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8/src/nspr-4.8.tar.gz
```

------------------------------
Bug
------------------------------

If a reader tries to recursively reacquire its reader lock after a writer has
shown up (and is waiting) deadlock will result.

Compare `src/rwlock-test.c` and `src/rwlock-test-sleep.c` to see the difference.


