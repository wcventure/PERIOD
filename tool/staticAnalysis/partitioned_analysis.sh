#!/bin/bash -e

ROOT_DIR=''

# For Mac
if [ $(command uname) = "Darwin" ]; then
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

NAME=$1
echo "[*] Making static analysis directory"
ANALYSIS_FILE=$(realpath "$1.bc")
DIR=$(dirname $ANALYSIS_FILE)
echo "[*] DIR:" $DIR
echo "[*] ROOT_DIR:" $ROOT_DIR
export PATH=${ROOT_DIR}/tool/SVF/Release-build/bin:$PATH

pushd $DIR >> /dev/null

if [ ! -s ./mssa.$NAME ]; then
    echo "[*] Generating mssa.$NAME"
    rm -f ./mssa.$NAME
    $ROOT_DIR/tool/staticAnalysis/analysis.py ./$NAME.bc > ./mssa.$NAME
fi

if [ ! -s ./mempair_all.$NAME ]; then
    echo "[*] Generating mempair_all.$NAME"
    rm -f ./mempair_all.$NAME
    $ROOT_DIR/tool/staticAnalysis/get_aliased_pair.py ./mssa.$NAME > ./mempair_all.$NAME
fi

if [ ! -s ./mempair.$NAME ]; then
    echo "[*] Prune and check_testing_bugs"
    rm -f ./mempair.$NAME
    $ROOT_DIR/tool/staticAnalysis/prune.py ./mempair_all.$NAME > ./mempair.$NAME
    #$ROOT_DIR/tool/staticAnalysis/check_testing_bugs.py ./mempair.$NAME
fi

ls -lh *$NAME*
popd 2>/dev/null
