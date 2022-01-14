#!/bin/bash

# For Mac
if [ $(command uname) == "Darwin" ]; then
	if ! [ -x "$(command -v greadlink)"  ]; then
		brew install coreutils
	fi
	BIN_PATH=$(greadlink -f "$0")
	ROOT_DIR=$(dirname $(dirname $(dirname $BIN_PATH)))
# For Linux
else
	BIN_PATH=$(readlink -f "$0")
	ROOT_DIR=$(dirname $(dirname $(dirname $BIN_PATH)))
fi

export ROOT_DIR=${ROOT_DIR}
export PATH=${ROOT_DIR}/clang+llvm/bin:${ROOT_DIR}/tool/SVF/Release-build/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

echo "Installation completed. Everything's fine!"

set -eux


# compile the program and get bit code
cd $ROOT_DIR/test/doubleFree/
./cleanDIR.sh
clang -g -emit-llvm -c ./df.c -o df.bc

# perform static analysis
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh df

# complie the instrumented program with ASAN
export Con_PATH=$ROOT_DIR/test/doubleFree/ConConfig.df
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast -g -fsanitize=address -c ./df.c -o df.o
clang++ ./df.o $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/DBDSFunction.o -g -o df -lpthread -fsanitize=address -ldl
