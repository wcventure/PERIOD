#!/bin/bash

# For Mac
if [ $(command uname) == "Darwin" ]; then
        if ! [ -x "$(command -v greadlink)"  ]; then
                brew install coreutils
        fi
        BIN_PATH=$(greadlink -f "$0")
        ROOT_DIR=$(dirname $(dirname $(dirname $(dirname $(dirname $BIN_PATH)))))
# For Linux
else
        BIN_PATH=$(readlink -f "$0")
        ROOT_DIR=$(dirname $(dirname $(dirname $(dirname $(dirname $BIN_PATH)))))
fi

set -e
set -u
DIR=`dirname $0`
COMMON_SH=$DIR/common.sh
if [ ! -e "$COMMON_SH" ]; then
    echo "ERROR: unable to find common.sh at $COMMON_SH"
    exit 1
fi

source $COMMON_SH

MY_SRC=$TEST_SRC 

OUTNAME=test-rw-wllvm

echo
echo "Building Test Harness..."
wllvm -o $BIN_DIR/$OUTNAME -fvisibility=hidden -pipe -Wall -pthread -g -fno-inline -fPIC  -UNDEBUG -DDEBUG_najalber  -DDEBUG=1 -DHAVE_VISIBILITY_HIDDEN_ATTRIBUTE=1 -DHAVE_VISIBILITY_PRAGMA=1 -DXP_UNIX=1 -D_GNU_SOURCE=1 -DHAVE_FCNTL_FILE_LOCKING=1 -DLINUX=1 -Di386=1 -DHAVE_LCHOWN=1 -DHAVE_STRERROR=1 -D_REENTRANT=1  -DFORCE_PR_LOG -D_PR_PTHREADS -UHAVE_CVAR_BUILT_ON_SEM -I$NSPR_DIST/include/nspr -I$NSPR_SRC/mozilla/nsprpub/pr/include -I$NSPR_SRC/mozilla/nsprpub/pr/include/private $TEST_SRC $NSPR_DIST/lib/libnspr4.a $NSPR_DIST/lib/libplc4.a $NSPR_DIST/lib/libplds4.a -lpthread -ldl

EXECUTE_CMD="$BIN_DIR/$OUTNAME"

echo "Test Harness Built!"
echo "To Execute, you must specify the shared library path:"
echo "  $EXECUTE_CMD"
echo "$EXECUTE_CMD" &> $BIN_DIR_README

cd $BIN_DIR
extract-bc $EXECUTE_CMD
rm $BIN_DIR/$OUTNAME.bc.r
rm $BIN_DIR/$OUTNAME
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh test-rw-wllvm
sed -i '/prrwlock.c/!d' ConConfig.test-rw-wllvm
mv $BIN_DIR/mempair.test-rw-wllvm.npz $BIN_DIR/mempair.main.npz
export Con_PATH=$BIN_DIR/ConConfig.test-rw-wllvm

$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g $BIN_DIR/test-rw-wllvm.bc -Xlinker -rpath $NSPR_DIST/lib -L$NSPR_DIST/lib -lplc4 -L$NSPR_DIST/lib -lnspr4 -lpthread -o $BIN_DIR/main
echo "Test Harness Built!"
echo "To Execute, you must specify the shared library path:"
echo "  $BIN_DIR/main"
