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

cd $ROOT_DIR/evaluation/NEW/pixz-1.0.7

if ! [ -d "${ROOT_DIR}/evaluation/NEW/pixz-1.0.7/pixz-1.0.7"  ]; then
	wget https://github.com/vasi/pixz/releases/download/v1.0.7/pixz-1.0.7.tar.gz
	tar xvf ./pixz-1.0.7.tar.gz
	rm -rf ./pixz-1.0.7.tar.gz
fi

./cleanDIR.sh
cd pixz-1.0.7
CC=wllvm CXX=wllvm++ CFLAGS="-g" CXXFLAGS="-g" ./configure
make
cd src
extract-bc ./pixz
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh pixz
export Con_PATH=$ROOT_DIR/evaluation/NEW/pixz-1.0.7/pixz-1.0.7/src/ConConfig.pixz
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -pthread -Wall -Wno-unknown-pragmas -g -fsanitize=address -o pixz pixz.bc -lm -larchive -llzma -lm
cd $ROOT_DIR/evaluation/NEW/pixz-1.0.7
# test
# $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 ./pixz-1.0.7/src/pixz -c -k -p 3 test.tar
