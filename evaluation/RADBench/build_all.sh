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
export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

echo "Everything's fine!"

set -eux

# bug2 src
cd $ROOT_DIR/evaluation/RADBench/bug2/src
if ! [ -f "${ROOT_DIR}/evaluation/RADBench/bug2/src/nspr-4.6.7.tar.gz"  ]; then
	wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.7/src/nspr-4.6.7.tar.gz
fi
if ! [ -f "${ROOT_DIR}/evaluation/RADBench/bug2/src/js-1.7.0.tar.gz"  ]; then
        wget https://ftp.mozilla.org/pub/mozilla.org/js/js-1.7.0.tar.gz
fi

# bug3 src
cd $ROOT_DIR/evaluation/RADBench/bug3/src
if ! [ -f "${ROOT_DIR}/evaluation/RADBench/bug3/src/nspr-4.7.3.tar.gz"  ]; then
        wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.7.3/src/nspr-4.7.3.tar.gz
fi
if ! [ -f "${ROOT_DIR}/evaluation/RADBench/bug3/src/js-1.8.0-rc1.tar.gz"  ]; then
	wget https://ftp.mozilla.org/pub/mozilla.org/js/js-1.8.0-rc1.tar.gz
fi

# bug4 src
cd $ROOT_DIR/evaluation/RADBench/bug4/src
if ! [ -f "${ROOT_DIR}/evaluation/RADBench/bug4/src/nspr-4.6.4.tar.gz"  ]; then
        wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.4/src/nspr-4.6.4.tar.gz
fi

# bug6 src
cd $ROOT_DIR/evaluation/RADBench/bug6/src
if ! [ -f "${ROOT_DIR}/evaluation/RADBench/bug6/src/nspr-4.8.tar.gz"  ]; then
        wget https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8/src/nspr-4.8.tar.gz
fi

# build bug2
cd $ROOT_DIR/evaluation/RADBench/bug2/
./cleanDIR.sh
make

# build bug3
cd $ROOT_DIR/evaluation/RADBench/bug3/
./cleanDIR.sh
make

# build bug4
cd $ROOT_DIR/evaluation/RADBench/bug4/
./cleanDIR.sh
make

# build bug6
cd $ROOT_DIR/evaluation/RADBench/bug6/
./cleanDIR.sh
make


