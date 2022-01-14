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

if [ ! -s ./ConConfig.$NAME ]; then
    echo "[*] Change format for instrumentation"
    rm -f ./ConConfig.$NAME
    ${ROOT_DIR}/tool/staticAnalysis/ExtractLineStdout.py -f ./mempair.$NAME > ./ConConfig.$NAME
fi

export Con_PATH=${DIR}/ConConfig.$NAME
if [ ! -s ./so.$NAME ]; then
    echo "[*] Analyze Sensitive Memory Operations"
    rm -f ./so.$NAME
    opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/SensitiveOperationsPass/libSensitiveOperationsPass.so -so ./$NAME.bc -o /dev/null > so.$NAME
fi

if [ ! -s ./mempairso.$NAME ]; then
    echo "[*] Update Mempair with Sensitive Memory Operations"
    rm -rf ./mempairso.$NAME
    $ROOT_DIR/tool/staticAnalysis/updateMempairSo.py -i ./so.$NAME -m ./mempair.$NAME > mempairso.$NAME
fi

#if [ ! -s ./dryRun.$NAME.bc ]; then
    #echo "[*] Perform dry run instrumentation"
    #rm -rf ./dryRun.$NAME.bc
    #opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DryRunPass/libDryRunPass.so -Instru ./$NAME.bc -o dryRun.$NAME.bc
#fi

#if [ ! -s ./dryRun.$NAME ]; then
    #echo "[*] Compile dry run program"
    #rm -rf ./dryRun.$NAME
    #clang++ -g -O0 ./dryRun.$NAME.bc $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DryRunPass/libinstruFunction.a -o dryRun.$NAME -lpthread -ldl -rdynamic
#fi

if [ ! -s ./dbds.$NAME.bc ]; then
    echo "[*] Perform dbds and dry run instrumentation"
    rm -rf ./dbds.$NAME.bc
    opt -load $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DBDSPass/libDBDSPass.so -Instru ./$NAME.bc -o dbds.$NAME.bc
fi

if [ ! -s ./dbds.$NAME ]; then
    echo "[*] Complie dbds progarm"
    rm -rf ./dbds.$NAME

    clang++ -g ./dbds.$NAME.bc $ROOT_DIR/tool/staticAnalysis/LLVM-PASS/DBDSPass/libDBDSFunction.a -o dbds.$NAME -lpthread -ldl -rdynamic
fi

ls -lh *$NAME*
popd 2>/dev/null
