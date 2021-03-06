# Test

Please try to perform following command:

```sh
# setup the environment variables in the root directory of the tool
$ source tool/init_env.sh

# compile the program and get bit code
$ cd $ROOT_DIR/evaluation/Splash2/barnes
$ ./cleanDIR.sh
$ CC=wllvm make
$ extract-bc ./BARNES

# perform static analysis
$ $ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh BARNES

# complie the instrumented program
$ export Con_PATH=$ROOT_DIR/evaluation/Inspect_benchmarks/barnes/ConConfig.BARNES
$ $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./BARNES.bc -o BARNES -lpthread -ldl

# perform DBDS
$ $ROOT_DIR/tool/DBDS/run_PDS.py --no-prefix -d 2 ./BARNES
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

# About BARNES

GENERAL INFORMATION:

The BARNES application implements the Barnes-Hut method to simulate the 
interaction of a system of bodies (N-body problem).  A general description 
of the Barnes-Hut method can be found in:

Singh, J. P.  Parallel Hierarchical N-body Methods and Their Implications
     for Multiprocessors.  PhD Thesis, Stanford University, February 1993.

The SPLASH-2 implementation allows for multiple particles to be stored in 
each leaf cell of the space partition.  A description of this feature 
can be found in:

Holt, C. and Singh, J. P.  Hierarchical N-Body Methods on Shared Address 
     Space Multiprocessors.  SIAM Conference on Parallel Processing
     for Scientific Computing, Feb 1995, to appear.

RUNNING THE PROGRAM:

To see how to run the program, please see the comment at the top of the
file code.C, or run the application with the "-h" command line option.
The input parameters should be placed in a file and redirected to standard 
input.  Of the twelve input parameters, the ones which would normally be 
varied are the number of particles and the number of processors.  If other 
parameters are changed, these changes should be reported in any results 
that are presented.

The only compile time option, -DQUADPOLE, controls the use of quadpole
interactions during the force computation.  For the input parameters
provided, the -DQUADPOLE option should not be defined.  The constant 
MAX_BODIES_PER_LEAF defines the maximum number of particles per leaf 
cell in the tree.  This constant also affects the parameter "fleaves" in 
the input file, which controls how many leaf cells space is allocated for.
The higher the value of MAX_BODIES_PER_LEAF, the lower fleaves should be.
Both these parameters should be kept at their default values for base
SPLASH-2 runs. If changes are made, they should be reported in any results 
that are presented.  

BASE PROBLEM SIZE:

The base problem size for an upto-64 processor machine is 16384 particles. 
For this many particles, you can use the input file provided (and change 
only the number of processors).

DATA DISTRIBUTION:

Our "POSSIBLE ENHANCEMENT" comments in the source code tell where one 
might want to distribute data and how. Data distribution, however, does 
not make much difference to performance on the Stanford DASH 
multiprocessor.
