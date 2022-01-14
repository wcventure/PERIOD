#!/bin/bash -e

ROOT_DIR=''

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

NAME=$1
echo "[*] Making static analysis directory"
ANALYSIS_FILE=$(realpath "$1.bc")
DIR=$(dirname $ANALYSIS_FILE)
echo "[*] DIR:" $DIR
echo "[*] ROOT_DIR:" $ROOT_DIR
export PATH=${ROOT_DIR}/tool/SVF/Release-build/bin:$PATH
export PATH=${ROOT_DIR}/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=${ROOT_DIR}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

pushd $DIR >> /dev/null

if [ ! -s ./mssa.$NAME ]; then
    echo "[*] Generating mssa.$NAME"
    rm -f ./mssa.$NAME
    $ROOT_DIR/tool/staticAnalysis/analysis.py ./$NAME.bc > ./mssa.$NAME
fi

if [ ! -s ./ConConfig.$NAME ]; then
    echo "[*] Change format for instrumentation"
    rm -f ./ConConfig.$NAME
    ${ROOT_DIR}/tool/staticAnalysis/get_aliased_info.py ./mssa.$NAME > ./ConConfig.$NAME
fi

export Con_PATH=${DIR}/ConConfig.$NAME
if [ ! -s ./so.$NAME ]; then
    echo "[*] Analyze Sensitive Memory Operations"
    rm -f ./so.$NAME
    opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/SensitiveOperationsPass/libSensitiveOperationsPass.so -so ./$NAME.bc -o /dev/null > so.$NAME
fi

rm -rf ./ConConfig.$NAME
if [ ! -s ./ConConfig.$NAME ]; then
    echo "[*] Update Memory Group with Sensitive Memory Operations"
    $ROOT_DIR/tool/staticAnalysis/updateMemGroupSo.py -i ./so.$NAME -m ./memGroup.npy > ./ConConfig.$NAME
fi
rm -rf ./memGroup.npy

if [ ! -s ./mempair.$NAME ]; then
    echo "[*] Dumps mempairs and save it into .npz file"
    rm -f ./mempair_all.$NAME
    $ROOT_DIR/tool/staticAnalysis/DumpMempair.py -o mempair.$NAME.npz > ./mempair.$NAME
fi
rm -rf ./soAndGroup.npz



ls -lh *$NAME*
popd 2>/dev/null
