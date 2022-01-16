#!/bin/bash

set -e
DIR=`dirname $0`
COMMON_SH=$DIR/common.sh
if [ ! -e "$COMMON_SH" ]; then
    echo "ERROR: unable to find common.sh at $COMMON_SH"
    exit 1
fi

source $COMMON_SH

set -u

if [ ! -d "$NSPR_SRC" ]; then 
    echo
    echo "Unzipping NSPR Source..."
    cd $SRC_DIR
    tar xzf $NSPR_ZIP
    if [ ! -d "$NSPR_SRC" ]; then
        echo "ERROR: unzip failed to create $NSPR_SRC"
        exit 1
    fi
    cp $NSPR_SRC/../os_Linux_x86.s $NSPR_SRC/mozilla/nsprpub/pr/src/md/
    cp $NSPR_SRC/../os_Linux_x86_64.s $NSPR_SRC/mozilla/nsprpub/pr/src/md/
    cp $NSPR_SRC/../os_Linux_x86.s $NSPR_SRC/mozilla/nsprpub/pr/src/md/unix/
    cp $NSPR_SRC/../os_Linux_x86_64.s $NSPR_SRC/mozilla/nsprpub/pr/src/md/unix/
fi

echo
echo "Starting Building NSPR..."
mkdir -p $NSPR_TARGET
cd $NSPR_TARGET
CC=wllvm CXX=wllvm++ ../mozilla/nsprpub/configure --enable-64bit
make
echo "NSPR installed to $NSPR_DIST"
