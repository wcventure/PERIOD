#!/bin/bash

# For Mac
if [ $(command uname) == "Darwin" ]; then
	if ! [ -x "$(command -v greadlink)"  ]; then
		brew install coreutils
	fi
	BIN_PATH=$(greadlink -f "$0")
	ROOT_DIR=$(dirname $(dirname $BIN_PATH))
# For Linux
else
	BIN_PATH=$(readlink -f "$0")
	ROOT_DIR=$(dirname $(dirname $BIN_PATH))
fi

set -euxo pipefail

if ! [ -d "${ROOT_DIR}/clang+llvm"  ]; then
	${ROOT_DIR}/tool/install_llvm.sh
fi

export ROOT_DIR=${ROOT_DIR}
export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export CC=clang
export CXX=clang++

cd ${ROOT_DIR}/tool/staticAnalysis/LLVM-PASS
rm -rf CMakeFiles
rm -rf CMakeCache.txt
rm -rf SensitiveOperationsPass/CMakeFiles
rm -rf SensitiveOperationsPass/CMakeCache.txt
cmake .
make

cd ${ROOT_DIR}/tool/staticAnalysis/DBDS-INSTRU/llvm-mode
make clean
make

#echo "Executing the Test Suite..."
#${ROOT_DIR}/tool/staticAnalysis/clean_shm.py > /dev/null
#Dry_Run=0 Run_DBDS=0 ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ${ROOT_DIR}/tool/staticAnalysis/DBDS-INSTRU/llvm-mode/test-instr > /dev/null 2>&1

echo "Installation completed. Everything's fine!"
