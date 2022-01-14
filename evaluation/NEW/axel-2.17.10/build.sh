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

cd ${ROOT_DIR}/evaluation/NEW/axel-2.17.10/

if ! [ -d "${ROOT_DIR}/evaluation/NEW/axel-2.17.10/axel-2.17.10"  ]; then
	wget https://github.com/axel-download-accelerator/axel/releases/download/v2.17.10/axel-2.17.10.tar.gz
	tar xvf ./axel-2.17.10.tar.gz
	rm -rf ./axel-2.17.10.tar.gz
fi

./cleanDIR.sh
cd $ROOT_DIR/evaluation/NEW/axel-2.17.10/axel-2.17.10/
CC=wllvm CXX=wllvm++ CFLAGS="-g -O0" CXXFLAGS="-g -O0" ./configure
make
extract-bc axel
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh axel
export Con_PATH=$ROOT_DIR/evaluation/NEW/axel-2.17.10/axel-2.17.10/ConConfig.axel
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -fsanitize=thread -o axel axel.bc -lpthread -ldl -lssl -lcrypto
# test
# $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -t 20 ./axel -n 4 -o test.pdf  https://www.carolemieux.com/fairfuzz-ase18.pdf && rm test.pdf
