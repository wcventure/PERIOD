#!/bin/bash

set -e
set -u
DIR=`dirname $0`
COMMON_SH=$DIR/common.sh
if [ ! -e "$COMMON_SH" ]; then
    echo "ERROR: unable to find common.sh at $COMMON_SH"
    exit 1
fi

source $COMMON_SH

OUTNAME=test-cvar

echo
echo "Building Test Harness..."
cd $BIN_DIR
extract-bc $BIN_DIR/$OUTNAME
rm $BIN_DIR/$OUTNAME.bc.r
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh $OUTNAME
sed -i '/ptsynch.c\|ptthread.c/!d' ConConfig.$OUTNAME
mv $BIN_DIR/mempair.$OUTNAME.npz $BIN_DIR/mempair.main.npz
export Con_PATH=$BIN_DIR/ConConfig.$OUTNAME
TEST_SRC=$BIN_DIR/$OUTNAME.bc
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ $TEST_SRC -Xlinker -rpath $NSPR_DIST/lib -L$NSPR_DIST/lib -lplc4 -L$NSPR_DIST/lib -lnspr4 -lpthread -o $BIN_DIR/main

EXECUTE_CMD="$BIN_DIR/main"

echo "Test Harness Built!"
echo "To Execute, you must specify the shared library path:"
echo "  $EXECUTE_CMD"
echo "$EXECUTE_CMD" &> $BIN_DIR_README

