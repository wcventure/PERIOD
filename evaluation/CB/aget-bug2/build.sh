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

cd $ROOT_DIR/evaluation/CB/aget-bug2/
./cleanDIR.sh
cd $ROOT_DIR/evaluation/CB/aget-bug2/aget-devel
CC=wllvm make
extract-bc ./aget
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh aget
export Con_PATH=$ROOT_DIR/evaluation/CB/aget-bug2/aget-devel/ConConfig.aget
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g ./aget.bc -o aget -lpthread -ldl -fsanitize=address
cd ..

