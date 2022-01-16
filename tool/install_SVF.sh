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

# set -euxo pipefail

if ! [ -d "${ROOT_DIR}/clang+llvm"  ]; then
	${ROOT_DIR}/tool/install_llvm.sh
fi

export ROOT_DIR=${ROOT_DIR}
export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export CC=clang
export CXX=clang++
export LLVM_HOME=${ROOT_DIR}/clang+llvm/bin
export LLVM_DIR=$LLVM_HOME
export PATH=$LLVM_HOME:$PATH

cd ${ROOT_DIR}/tool/SVF
source ./build.sh

rm -rf ${ROOT_DIR}/tool/Release-build
${ROOT_DIR}/tool/SVF/Release-build/bin/wpa --version
echo "Finish SVF installation"

