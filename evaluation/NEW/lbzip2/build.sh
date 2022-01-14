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
export PATH=${ROOT_DIR}/clang+llvm/bin:${ROOT_DIR}/tool/SVF/Release-build/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

echo "Environment variables is OK. Everything's fine!"

set -eux

cd ${ROOT_DIR}/evaluation/NEW/lbzip2/
./cleanDIR.sh
cd $ROOT_DIR/evaluation/NEW/lbzip2/lbzip2-2.5/
./build-aux/autogen.sh
CC=wllvm CXX=wllvm++ CFLAGS="-g -O0" CXXFLAGS="-g -O0" ./configure
make
cd src
extract-bc ./lbzip2
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh lbzip2
sed -i '/encode.c:/d' ConConfig.lbzip2
sed -i '/decode.c:/d' ConConfig.lbzip2
export Con_PATH=$ROOT_DIR/evaluation/NEW/lbzip2/lbzip2-2.5/src/ConConfig.lbzip2
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -o lbzip2 lbzip2.bc -lpthread -fsanitize=thread
# test
# cd ${ROOT_DIR}/evaluation/NEW/lbzip2/
# $ROOT_DIR/tool/DBDS/run_PDS.py -d 2 lbzip2-2.5/src/lbzip2 -k -t -1 -d -f -n2 lbzip2-2.5/tests/32767.bz2