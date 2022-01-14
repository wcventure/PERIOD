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

echo "$ROOT_DIR"
export ROOT_DIR=${ROOT_DIR}
export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

echo "Everything's fine!"

set -eux

# compile the program and get bit code
cd $ROOT_DIR/evaluation/CB/stringbuffer-jdk1.4
./cleanDIR.sh
CXX=wllvm++ make
extract-bc ./main

# perform static analysis
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh main

# complie the instrumented program with ASAN
export Con_PATH=$ROOT_DIR/evaluation/CB/stringbuffer-jdk1.4/ConConfig.main
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=address ./main.bc -o main -lpthread -ldl

cd ..

