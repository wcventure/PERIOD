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

cd ${ROOT_DIR}/evaluation/NEW/xz/

if ! [ -d "${ROOT_DIR}/evaluation/NEW/xz/xz-5.2.5"  ]; then
	wget https://nchc.dl.sourceforge.net/project/lzmautils/xz-5.2.5.tar.gz
	cd xz-5.2.5
fi

set -ux

cd ${ROOT_DIR}/evaluation/NEW/xz
./cleanDIR.sh
cd $ROOT_DIR/evaluation/NEW/xz/xz-5.2.5
CC=wllvm CXX=wllvm++ CFLAGS="-g -O0" CXXFLAGS="-g -O0" ./configure
make
cd src/xz/.libs
extract-bc ./xz
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh xz
export Con_PATH=$ROOT_DIR/evaluation/NEW/xz/xz-5.2.5/src/xz/.libs/ConConfig.xz
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -pthread -fvisibility=hidden -Wall -Wextra -Wvla -Wformat=2 -Winit-self -Wmissing-include-dirs -Wstrict-aliasing -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn -Wredundant-decls -o xz xz.bc ../../../src/liblzma/.libs/liblzma.so -lpthread -fsanitize=thread 
# test
# $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -t 5 ./xz-5.2.5/src/xz/.libs/xz -9 -k -T 4 -f $FILE