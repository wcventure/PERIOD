# Test

Use the script `build.sh` (The command `source tool/init_env.sh` is need whenever you open a new terminal). Please try to perform following command:

```sh
# compile the program, perform static analysis, and perform instrumentation
$ cd $ROOT_DIR/evaluation/NEW/lrzip
$ ./build.sh

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py -d 1 -t 10 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:$LD_PRELOAD lrzip/lrzip -t -p2 ./POC
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

# Bug Report 

Dear all, 

Our tool report that there would be multiple concurrency use-after-free between `zpaq_decompress_buf()` function and `clear_rulist()` function, in the newest master branch `465afe8`.

### Brief Explanation

The related code simplified from `stream.c` and `runzip.c` are shown as follow:

```C
// Thread T0                                | // Thread T1
// in clear_rulist() in runzip.c            | // in zpaq_decompress_buf() in steam.c
...                                         | ...
struct runzip_node *node = control->ruhead; | if (unlikely(dlen != ucthread->u_len)) {
struct stream_info *sinfo = node->sinfo;    |     ret = -1;
                                            | } else
dealloc(sinfo->ucthreads);                  |     dealloc(c_buf);
dealloc(sinfo);                             | out:
                                            |     if (ret == -1) {
                                            |         dealloc(ucthread->s_buf);
		                                    |         ucthread->s_buf = c_buf;
	                                        |     }
```

Both thread T0 and thread T1 operate on a shared variable `ucthread` (i.e., T0 dealloc the a ucthread through `dealloc(sinfo->ucthreads);`, and T1 use the ucthread in all statements `if (unlikely(dlen != ucthread->u_len))`, `dealloc(ucthread->s_buf);`, and `ucthread->s_buf = c_buf;`).
However, a use-after-free can occur if the deallocation of ucthread before the use of ucthread.
For example, the following three thread interleaving can trigger three different UAFs:

*Interleaving (a)*
```C

// Thread T0                                | // Thread T1
                                            |
...                                         | 
struct runzip_node *node = control->ruhead; | 
struct stream_info *sinfo = node->sinfo;    | 
                                            | 
dealloc(sinfo->ucthreads);                  | 
dealloc(sinfo);                             | 
----------------------------------------------------------------------------------------------------
	                                        | ...
                                            | if (unlikely(dlen != ucthread->u_len)) { // UAF here
                                            |     ret = -1;
                                            | } else
                                            |     dealloc(c_buf);
                                            | out:
                                            |     if (ret == -1) {
                                            |         dealloc(ucthread->s_buf);
		                                    |         ucthread->s_buf = c_buf;
	                                        |     }

```

*Interleaving (b)*
```C
// Thread T0                                | // Thread T1
                                            |
                                            | ...
                                            | if (unlikely(dlen != ucthread->u_len)) {
                                            |     ret = -1;
                                            | } else
                                            |     dealloc(c_buf);
                                            | out:
                                            |     if (ret == -1) {
----------------------------------------------------------------------------------------------------
...                                         | 
struct runzip_node *node = control->ruhead; | 
struct stream_info *sinfo = node->sinfo;    | 
                                            | 
dealloc(sinfo->ucthreads);                  | 
dealloc(sinfo);                             | 
----------------------------------------------------------------------------------------------------
                                            |         dealloc(ucthread->s_buf);  // UAF occur here
		                                    |         ucthread->s_buf = c_buf;
	                                        |     }
```

*Interleaving (c)*
```C
// Thread T0                                | // Thread T1
                                            |
                                            | ...
                                            | if (unlikely(dlen != ucthread->u_len)) {
                                            |     ret = -1;
                                            | } else
                                            |     dealloc(c_buf);
                                            | out:
                                            |     if (ret == -1) {
                                            |         dealloc(ucthread->s_buf); 
----------------------------------------------------------------------------------------------------
...                                         | 
struct runzip_node *node = control->ruhead; | 
struct stream_info *sinfo = node->sinfo;    | 
                                            | 
dealloc(sinfo->ucthreads);                  | 
dealloc(sinfo);                             | 
----------------------------------------------------------------------------------------------------
		                                    |         ucthread->s_buf = c_buf; // UAF occur here
	                                        |     }
```


### Reproduce through delay injection

To reproduce those use-after-free errors, we can insert two delays (e.g., `sleep(1)`) into the original source code.

For example, to reproduce interleaving (a) as mentioned earlier, you can insert a delay before `dealloc(sinfo->ucthreads);` statement in function in `steam.c`, and also a delay after, as shown as follows.

```C
// In runzip.c, insert a delay after `dealloc(sinfo->ucthreads);`
static void clear_rulist(rzip_control *control)
{
	while (control->ruhead) {
		struct runzip_node *node = control->ruhead;
		struct stream_info *sinfo = node->sinfo;
	
		dealloc(sinfo->ucthreads);
		sleep(1); // delay here !!!!!!!!!!
		dealloc(node->pthreads);
		dealloc(sinfo->s);
		dealloc(sinfo);
		control->ruhead = node->prev;
		dealloc(node);
	}
}

// In steam.c, insert a delay after `dealloc(sinfo->ucthreads);`
static int zpaq_decompress_buf(rzip_control *control __UNUSED__, struct uncomp_thread *ucthread, long thread)
{
	...
	zpaq_decompress(ucthread->s_buf, &dlen, c_buf, ucthread->c_len,
			control->msgout, SHOW_PROGRESS ? true: false, thread);
	sleep(1); // delay here !!!!!!!!!!
	if (unlikely(dlen != ucthread->u_len)) {
		print_err("Inconsistent length after decompression. Got %ld bytes, expected %lld\n", dlen, ucthread->u_len);
		ret = -1;
	} else
		dealloc(c_buf);
    ...
}

```

compile the program:
```shell
CC=gcc CXX=g++ CFLAGS="-g -O0 -fsanitize=address" CXXFLAGS="-g -O0 -fsanitize=address" ./configure --enable-static-bin
make
```
  
Download the testcase (I upload the POC here, please unzip first).


Run with the testcase with following command:

```shell
./lrzip -t -p2 POC
```

Then, you will see the use-after-free bug report. Here is the trace reported by ASAN:

```shell
=================================================================
==33325==ERROR: AddressSanitizer: heap-use-after-free on address 0x61d0000000e8 at pc 0x000000512b00 bp 0x7f9a30bfdb10 sp 0x7f9a30bfdb08

READ of size 8 at 0x61d0000000e8 thread T3
    #0 0x512aff in zpaq_decompress_buf /workdir/lrzip/stream.c:449:6
    #1 0x510381 in ucompthread /workdir/lrzip/stream.c:1554:11
    #2 0x7f9a3541b6da in start_thread (/lib/x86_64-linux-gnu/libpthread.so.0+0x76da)
    #3 0x7f9a3479771e in clone (/lib/x86_64-linux-gnu/libc.so.6+0x12171e)

0x61d0000000e8 is located 104 bytes inside of 2400-byte region [0x61d000000080,0x61d0000009e0)
freed by thread T0 here:
    #0 0x494e1d in free /home/brian/src/final/llvm-project/compiler-rt/lib/asan/asan_malloc_linux.cpp:123:3
    #1 0x4faa0d in clear_rulist /workdir/lrzip/runzip.c:255:3
    #2 0x4f7ab2 in runzip_chunk /workdir/lrzip/runzip.c:384:2
    #3 0x4f4aae in runzip_fd /workdir/lrzip/runzip.c:404:7
    #4 0x4d84ce in decompress_file /workdir/lrzip/lrzip.c:845:6
    #5 0x4cb98b in main /workdir/lrzip/main.c:706:4
    #6 0x7f9a34697bf6 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x21bf6)

previously allocated by thread T0 here:
    #0 0x495212 in calloc /home/brian/src/final/llvm-project/compiler-rt/lib/asan/asan_malloc_linux.cpp:154:3
    #1 0x5003e1 in open_stream_in /workdir/lrzip/stream.c:1084:33
    #2 0x4f6fa5 in runzip_chunk /workdir/lrzip/runzip.c:322:7
    #3 0x4f4aae in runzip_fd /workdir/lrzip/runzip.c:404:7
    #4 0x4d84ce in decompress_file /workdir/lrzip/lrzip.c:845:6
    #5 0x4cb98b in main /workdir/lrzip/main.c:706:4
    #6 0x7f9a34697bf6 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x21bf6)

Thread T3 created by T0 here:
    #0 0x47fe4a in pthread_create /home/brian/src/final/llvm-project/compiler-rt/lib/asan/asan_interceptors.cpp:214:3
    #1 0x4fbbd0 in create_pthread /workdir/lrzip/stream.c:125:6
    #2 0x507033 in fill_buffer /workdir/lrzip/stream.c:1713:6
    #3 0x504184 in read_stream /workdir/lrzip/stream.c:1800:8
    #4 0x4f9c88 in unzip_literal /workdir/lrzip/runzip.c:162:16
    #5 0x4f731c in runzip_chunk /workdir/lrzip/runzip.c:338:9
    #6 0x4f4aae in runzip_fd /workdir/lrzip/runzip.c:404:7
    #7 0x4d84ce in decompress_file /workdir/lrzip/lrzip.c:845:6
    #8 0x4cb98b in main /workdir/lrzip/main.c:706:4
    #9 0x7f9a34697bf6 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x21bf6)

SUMMARY: AddressSanitizer: heap-use-after-free /workdir/lrzip/stream.c:449:6 in zpaq_decompress_buf
Shadow bytes around the buggy address:
  0x0c3a7fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c3a7fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c3a7fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c3a7fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c3a7fff8000: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>0x0c3a7fff8010: fd fd fd fd fd fd fd fd fd fd fd fd fd[fd]fd fd
  0x0c3a7fff8020: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c3a7fff8030: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c3a7fff8040: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c3a7fff8050: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c3a7fff8060: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
  Shadow gap:              cc
==33325==ABORTING
```

I'm not sure if these use-after-free bugs could cause serious harm. I hope you can check whether it is necessary to fix these bugs.

Thanks.