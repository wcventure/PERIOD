# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Splash2/fft
$ ./cleanDIR.sh
$ CC=wllvm make
$ extract-bc ./FFT

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh FFT

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Splash2/fft/ConConfig.FFT
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./FFT.bc -o FFT -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 2 LOOP_UNWINDED_NUM=3 ./FFT -p2 -m2 -n4 -l4
```

Then you will get the results.

```sh
Start Testing!
test 0001
test 0002
...
```

The Assertion failure:

```sh
```

# About FFT

GENERAL INFORMATION:

The FFT program is a complex, one-dimensional version of the "Six-Step" 
FFT described in 

Bailey, D. H. FFTs in External or Hierarchical Memory. 
     Journal of Supercomputing, 4(1):23-35, March 1990.  
     
Specific optimizations in this implementation include: (1) Performing 
staggered, blocked transposes that exploit cache-line reuse; (2) The 
roots of unity data structure is arranged and distributed for only local
accesses during application of the roots of unity step; (3) A small set of 
roots of unity elements are replicated locally at each processor for 
computation of the 1D FFTs; and (4) The matrix data structures are padded 
to reduce cache mapping conflicts.  The algorithm used in this 
implementation is described in:

Woo, S. C., Singh, J. P., and Hennessy, J. L.  The Performance Advantages
     of Integrating Block Data Transfer in Cache-Coherent Multiprocessors.
     Proceedings of the 6th International Conference on Architectural
     Support for Programming Languages and Operating Systems (ASPLOS-VI),
     October 1994.

This program works under both the Unix FORK and SPROC models.

RUNNING THE PROGRAM:

To see how to run the program, please see the comment at the top of the 
file fft.C, or run the application with the "-h" command line option.
Four command-line parameters MUST be specified: The number of points
to transform, the number of processors, the log base 2 of the cache
line size, and the number of cache lines.  

The number of complex data points to be transformed and the number of 
processors being used can be varied according to the following rules.  
The number of data points must be an even power of 2 (2**16, 2**18, 
etc).  The command-line option "-mM" specifies the even power of two (M), 
so for 65,536 data points, the command line option is "-m16."  The number 
of processors must be a power of 2.  

The main data structures are padded to reduce cache mapping conflicts.  
The constant PAGE_SIZE should be changed to reflect the page size (in 
bytes) of the target system.  In addition, the program uses a blocking 
technique to exploit cache line reuse.  Both the log base 2 of the cache 
line size and the number of cache lines in the cache should be specified 
at the command line in order to allow the blocking algorithm to work 
effectively.  

BASE PROBLEM SIZE:

The base problem size for an upto-64 processor machine is 65,536 complex
data points (M=16).  The PAGE_SIZE constant should be changed to reflect
the page size (in bytes) of the target system.  In addition, the log base
2 of the cache line size, the number of cache lines, and the number of
processors must be specified.

DATA DISTRIBUTION:

Our "POSSIBLE ENHANCEMENT" comments in the source code tell where one
might want to distribute data and how.  Data distribution has an impact
on performance on the Stanford DASH multiprocessor.

