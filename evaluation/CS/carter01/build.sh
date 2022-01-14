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
cd $ROOT_DIR/evaluation/CS/carter01
./cleanDIR.sh
clang -g -emit-llvm -c ./carter01_bad.c -o carter01_bad.bc

# perform static analysis
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh carter01_bad

# complie the instrumented program
export Con_PATH=$ROOT_DIR/evaluation/CS/carter01/ConConfig.carter01_bad
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./carter01_bad.bc -o carter01_bad -lpthread -ldl



