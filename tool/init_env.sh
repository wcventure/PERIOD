#!/usr/bin/env bash

DIR_BACKUP=$(pwd)
cd $(dirname $BASH_SOURCE)
ROOT_DIR="$(dirname `pwd`)"
cd $DIR_BACKUP

read -n 1 -p "Is this your tool's root directory: $ROOT_DIR? (Y/n): " c
echo ""

if [[ $c = n ]] || [[ $c = N ]]; then
    read -p "Define your tool's root directory here: " ROOT_DIR
fi

echo "Root directory is set to be: $ROOT_DIR"
export ROOT_DIR
export PATH=${ROOT_DIR}/clang+llvm/bin:${ROOT_DIR}/tool/SVF/Release-build/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
echo "Environment initiation done!"
