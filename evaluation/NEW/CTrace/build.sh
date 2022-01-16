#!/bin/bash

# For Mac
if [ $(command uname) == "Darwin" ]; then
	if ! [ -x "$(command -v greadlink)"  ]; then
		brew install coreutils
	fi
	BIN_PATH=$(greadlink -f "$0")
	ROOT_DIR=$(dirname $(dirname $(dirname $(dirname $BIN_PATH))))
# For Linux
else
	BIN_PATH=$(readlink -f "$0")
	ROOT_DIR=$(dirname $(dirname $(dirname $(dirname $BIN_PATH))))
fi

export ROOT_DIR=${ROOT_DIR}
export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

echo "Everything's fine!"

set -eux

# build tests

# compile the program and get bit code
cd $ROOT_DIR/evaluation/NEW/CTrace
./cleanDIR.sh
CC=wllvm CFLAGS="-g -O0" make
cd $ROOT_DIR/evaluation/NEW/CTrace/obj
extract-bc ctrace.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh ctrace.o

export Con_PATH=$ROOT_DIR/evaluation/NEW/CTrace/obj/ConConfig.ctrace.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -c ./ctrace.o.bc -o ctrace.o -lpthread -ldl -fsanitize=thread

cd $ROOT_DIR/evaluation/NEW/CTrace
ar rus lib/libctrace.a obj/*.o

cd test
wllvm -I../inc -L../lib -g -O0 -c main.c
extract-bc ./main.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -O0 -c ./main.o.bc -o main.o
clang++ -o test -I../inc -L../lib -g  main.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o ../obj/ctrace.o -lpthread -ldl -fsanitize=thread

# cd test
# $ROOT_DIR/tool/DBDS/run_PDS.py ./test