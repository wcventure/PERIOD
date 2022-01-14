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

cd ${ROOT_DIR}/evaluation/NEW/lrzip/

if ! [ -d "${ROOT_DIR}/evaluation/NEW/lrzip/lrzip"  ]; then
	git clone https://github.com/ckolivas/lrzip.git lrzip
	cd lrzip
	git checkout 465afe8
	cd $ROOT_DIR/evaluation/NEW/lrzip/lrzip/
	sed -i '305a{' runzip.c
	sed -i '306asleep(1);' runzip.c
	sed -i '308a}' runzip.c
	sed -i '386c //clear_rulist(control);' runzip.c
	sed -i '386ausleep(50000);' runzip.c
	sed -i '387apthread_t Dele;' runzip.c
	sed -i '388aif (unlikely(!create_pthread(control, &Dele, NULL, clear_rulist, control))) {' runzip.c
	sed -i '389areturn -1;' runzip.c
	sed -i '390a}' runzip.c
	sed -i '391apthread_join(Dele, NULL);' runzip.c
fi

set -ux

cd ${ROOT_DIR}/evaluation/NEW/lrzip/
./cleanDIR.sh
cd $ROOT_DIR/evaluation/NEW/lrzip/lrzip/
./autogen.sh
CC=wllvm CXX=wllvm++ CFLAGS="-g -O0" CXXFLAGS="-g -O0" ./configure --enable-static-bin
make
extract-bc ./lrzip
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh lrzip
sed -i '/stream.c\|runzip.c/!d' ConConfig.lrzip
sed -i '1astream.c:449' ConConfig.lrzip
sed -i '1astream.c:456' ConConfig.lrzip
sed -i '1astream.c:457' ConConfig.lrzip
sed -i '1arunzip.c:255' ConConfig.lrzip
sed -i '1arunzip.c:256' ConConfig.lrzip

make distclean
rm -rf *.bc
CC=wllvm CXX=wllvm++ CFLAGS="-g -O0 -fsanitize=address" CXXFLAGS="-g -O0 -fsanitize=address" ./configure --enable-static-bin
make
extract-bc ./lrzip
export Con_PATH=$ROOT_DIR/evaluation/NEW/lrzip/lrzip/ConConfig.lrzip
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g  -o lrzip lrzip.bc -lpthread -lm -lz -lbz2 -llzo2 -llz4 -lasan
# test
# $ROOT_DIR/tool/DBDS/run_PDS.py -d 3 -t 5 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:$LD_PRELOAD ./lrzip -t -p2 ../POC