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

cd $ROOT_DIR/evaluation/NEW/pbzip2-1.0.5
./cleanDIR.sh
cd $ROOT_DIR/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src
./cleanDIR.sh
CXX=wllvm++ make
extract-bc ./pbzip2
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh pbzip2
export Con_PATH=$ROOT_DIR/evaluation/NEW/pbzip2-1.0.5/pbzip2-1.0.5-src/ConConfig.pbzip2
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=address -O2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_STACKSIZE_CUSTOMIZATION -pthread -D_POSIX_PTHREAD_SEMANTICS -Wno-reserved-user-defined-literal -Wno-c++11-narrowing pbzip2.cpp -o pbzip2 -lbz2 -lpthread
cd ..

