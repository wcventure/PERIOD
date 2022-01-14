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

cd ${ROOT_DIR}/evaluation/NEW/axel-2.17.10/
rm -rf ./mempair.*
rm -rf ./mempair_all.*
rm -rf ./mssa.*
rm -rf ./ConConfig.*
rm -rf ./out_*
rm -rf ./in
rm -rf ./*.dot
rm -rf ./*.bc.r
rm -rf ./.*.o
rm -rf ./.*.o.bc
rm -rf ./*.out
rm -rf ./*.bc
rm -rf ./so.*
rm -rf ./mempairso.*
rm -rf ./dryRun.*
rm -rf ./*.log
rm -rf ./dbds.*
rm -rf ./*.o
rm -rf ./*.npy
rm -rf ./*.npz
rm -rf Makefile
rm -rf axel
rm -rf axel.bc.r
rm -rf config.h
rm -rf config.log
rm -rf config.status
rm -rf lib/.deps/
rm -rf lib/.dirstamp
rm -rf po/Makefile
rm -rf po/Makefile.in
rm -rf po/POTFILES
rm -rf src/.deps/
rm -rf src/.dirstamp
rm -rf stamp-h1

cd ${ROOT_DIR}/evaluation/NEW/axel-2.17.10/axel-2.17.10/
rm -rf ./mempair.*
rm -rf ./mempair_all.*
rm -rf ./mssa.*
rm -rf ./ConConfig.*
rm -rf ./out_*
rm -rf ./in
rm -rf ./*.dot
rm -rf ./*.bc.r
rm -rf ./.*.o
rm -rf ./.*.o.bc
rm -rf ./*.out
rm -rf ./*.bc
rm -rf ./so.*
rm -rf ./mempairso.*
rm -rf ./dryRun.*
rm -rf ./*.log
rm -rf ./dbds.*
rm -rf ./*.o
rm -rf ./*.npy
rm -rf ./*.npz
rm -rf Makefile
rm -rf axel
rm -rf axel.bc.r
rm -rf config.h
rm -rf config.log
rm -rf config.status
rm -rf lib/.deps/
rm -rf lib/.dirstamp
rm -rf po/Makefile
rm -rf po/Makefile.in
rm -rf po/POTFILES
rm -rf src/.deps/
rm -rf src/.dirstamp
rm -rf stamp-h1