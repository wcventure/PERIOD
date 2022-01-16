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

set -eux

PREFIX=${ROOT_DIR}

if [ $(command uname) == "Darwin" ]; then
	# For Mac
	LLVM_VER=${LLVM_VER:-6.0.0}
	LINUX_VER=${LINUX_VER:-darwin}
	TAR_NAME=clang+llvm-${LLVM_VER}-x86_64-apple-${LINUX_VER}
else
	# For Linux
	LINUX_VER=${LINUX_VER:-ubuntu-16.04}
	#LLVM_VER=${LLVM_VER:-8.0.0}
	LLVM_VER=${LLVM_VER:-4.0.0}
	TAR_NAME=clang+llvm-${LLVM_VER}-x86_64-linux-gnu-${LINUX_VER}
fi
LLVM_DEP_URL=https://releases.llvm.org/${LLVM_VER}

#wget -c ${LLVM_DEP_URL}/${TAR_NAME}.tar.xz
#tar -C ${PREFIX} -xf ${TAR_NAME}.tar.xz
#rm ${TAR_NAME}.tar.xz
#rm -rf ${PREFIX}/clang+llvm
#mv ${PREFIX}/${TAR_NAME} ${PREFIX}/clang+llvm

if [ $(command uname) = "Darwin" ]; then
	# For Mac
	wget -c https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-apple-darwin.tar.xz
	tar -C ${PREFIX} -xf clang+llvm-10.0.0-x86_64-apple-darwin.tar.xz
	rm clang+llvm-10.0.0-x86_64-apple-darwin.tar.xz
	rm -rf ${PREFIX}/clang+llvm
	mv ${PREFIX}/clang+llvm-10.0.0-x86_64-apple-darwin ${PREFIX}/clang+llvm

else
	# For Linux
	wget -c https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
	tar -C ${PREFIX} -xf clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
	rm clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
	rm -rf ${PREFIX}/clang+llvm
	mv ${PREFIX}/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04 ${PREFIX}/clang+llvm
fi

set +x
