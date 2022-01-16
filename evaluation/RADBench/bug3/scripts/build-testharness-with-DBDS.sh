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


OBJ_OUTPUT=`ls $JS_SRC_SRC | grep OBJ`
if [ "$OBJ_OUTPUT" == "" ]; then
    echo "ERROR: unable to find JS engine compile"
    exit 1
fi
OBJ_OUTPUT=$JS_SRC_SRC/$OBJ_OUTPUT

#instrument lib
cd $OBJ_OUTPUT
extract-bc $OBJ_OUTPUT/jsgc.o
rm $OBJ_OUTPUT/jsgc.o.bc.r
mv $OBJ_OUTPUT/jsgc.o $OBJ_OUTPUT/jsgc_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsgc.o # jsgc.c:3469
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsgc.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsgc.o.bc -o $OBJ_OUTPUT/jsgc.o -fPIC

extract-bc $OBJ_OUTPUT/jscntxt.o
rm $OBJ_OUTPUT/jscntxt.o.bc.r
mv $OBJ_OUTPUT/jscntxt.o $OBJ_OUTPUT/jscntxt_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jscntxt.o
mv $OBJ_OUTPUT/mempair.jscntxt.o.npz $BIN_DIR/../mempair.lib.npz
export Con_PATH=$OBJ_OUTPUT/ConConfig.jscntxt.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jscntxt.o.bc -o $OBJ_OUTPUT/jscntxt.o -fPIC

extract-bc $OBJ_OUTPUT/jskwgen.o
rm $OBJ_OUTPUT/jskwgen.o.bc.r
mv $OBJ_OUTPUT/jskwgen.o $OBJ_OUTPUT/jskwgen_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jskwgen.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jskwgen.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jskwgen.o.bc -o $OBJ_OUTPUT/jskwgen.o -fPIC

extract-bc $OBJ_OUTPUT/jsapi.o
rm $OBJ_OUTPUT/jsapi.o.bc.r
mv $OBJ_OUTPUT/jsapi.o $OBJ_OUTPUT/jsapi_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsapi.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsapi.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsapi.o.bc -o $OBJ_OUTPUT/jsapi.o -fPIC

extract-bc $OBJ_OUTPUT/jsexn.o
rm $OBJ_OUTPUT/jsexn.o.bc.r
mv $OBJ_OUTPUT/jsexn.o $OBJ_OUTPUT/jsexn_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsexn.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsexn.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsexn.o.bc -o $OBJ_OUTPUT/jsexn.o -fPIC

extract-bc $OBJ_OUTPUT/jsarena.o
rm $OBJ_OUTPUT/jsarena.o.bc.r
mv $OBJ_OUTPUT/jsarena.o $OBJ_OUTPUT/jsarena_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsarena.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsarena.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsarena.o.bc -o $OBJ_OUTPUT/jsarena.o -fPIC

extract-bc $OBJ_OUTPUT/jsarray.o
rm $OBJ_OUTPUT/jsarray.o.bc.r
mv $OBJ_OUTPUT/jsarray.o $OBJ_OUTPUT/jsarray_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsarray.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsarray.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsarray.o.bc -o $OBJ_OUTPUT/jsarray.o -fPIC

extract-bc $OBJ_OUTPUT/jsatom.o
rm $OBJ_OUTPUT/jsatom.o.bc.r
mv $OBJ_OUTPUT/jsatom.o $OBJ_OUTPUT/jsatom_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsatom.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsatom.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsatom.o.bc -o $OBJ_OUTPUT/jsatom.o -fPIC

extract-bc $OBJ_OUTPUT/jsbool.o
rm $OBJ_OUTPUT/jsbool.o.bc.r
mv $OBJ_OUTPUT/jsbool.o $OBJ_OUTPUT/jsbool_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsbool.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsbool.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsbool.o.bc -o $OBJ_OUTPUT/jsbool.o -fPIC

extract-bc $OBJ_OUTPUT/jsdate.o
rm $OBJ_OUTPUT/jsdate.o.bc.r
mv $OBJ_OUTPUT/jsdate.o $OBJ_OUTPUT/jsdate_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsdate.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsdate.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsdate.o.bc -o $OBJ_OUTPUT/jsdate.o -fPIC

extract-bc $OBJ_OUTPUT/jsdbgapi.o
rm $OBJ_OUTPUT/jsdbgapi.o.bc.r
mv $OBJ_OUTPUT/jsdbgapi.o $OBJ_OUTPUT/jsdbgapi_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsdbgapi.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsdbgapi.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsdbgapi.o.bc -o $OBJ_OUTPUT/jsdbgapi.o -fPIC

extract-bc $OBJ_OUTPUT/jsdhash.o
rm $OBJ_OUTPUT/jsdhash.o.bc.r
mv $OBJ_OUTPUT/jsdhash.o $OBJ_OUTPUT/jsdhash_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsdhash.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsdhash.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsdhash.o.bc -o $OBJ_OUTPUT/jsdhash.o -fPIC

extract-bc $OBJ_OUTPUT/jsdtoa.o
rm $OBJ_OUTPUT/jsdtoa.o.bc.r
mv $OBJ_OUTPUT/jsdtoa.o $OBJ_OUTPUT/jsdtoa_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsdtoa.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsdtoa.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsdtoa.o.bc -o $OBJ_OUTPUT/jsdtoa.o -fPIC

extract-bc $OBJ_OUTPUT/jsemit.o
rm $OBJ_OUTPUT/jsemit.o.bc.r
mv $OBJ_OUTPUT/jsemit.o $OBJ_OUTPUT/jsemit_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsemit.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsemit.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsemit.o.bc -o $OBJ_OUTPUT/jsemit.o -fPIC

extract-bc $OBJ_OUTPUT/jsfun.o
rm $OBJ_OUTPUT/jsfun.o.bc.r
mv $OBJ_OUTPUT/jsfun.o $OBJ_OUTPUT/jsfun_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsfun.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsfun.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsfun.o.bc -o $OBJ_OUTPUT/jsfun.o -fPIC

extract-bc $OBJ_OUTPUT/jshash.o
rm $OBJ_OUTPUT/jshash.o.bc.r
mv $OBJ_OUTPUT/jshash.o $OBJ_OUTPUT/jshash_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jshash.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jshash.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jshash.o.bc -o $OBJ_OUTPUT/jshash.o -fPIC

extract-bc $OBJ_OUTPUT/jsinterp.o
rm $OBJ_OUTPUT/jsinterp.o.bc.r
mv $OBJ_OUTPUT/jsinterp.o $OBJ_OUTPUT/jsinterp_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsinterp.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsinterp.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsinterp.o.bc -o $OBJ_OUTPUT/jsinterp.o -fPIC

extract-bc $OBJ_OUTPUT/jsiter.o
rm $OBJ_OUTPUT/jsiter.o.bc.r
mv $OBJ_OUTPUT/jsiter.o $OBJ_OUTPUT/jsiter_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsiter.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsiter.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsiter.o.bc -o $OBJ_OUTPUT/jsiter.o -fPIC

extract-bc $OBJ_OUTPUT/jslock.o
rm $OBJ_OUTPUT/jslock.o.bc.r
mv $OBJ_OUTPUT/jslock.o $OBJ_OUTPUT/jslock_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jslock.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jslock.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jslock.o.bc -o $OBJ_OUTPUT/jslock.o -fPIC

extract-bc $OBJ_OUTPUT/jslog2.o
rm $OBJ_OUTPUT/jslog2.o.bc.r
mv $OBJ_OUTPUT/jslog2.o $OBJ_OUTPUT/jslog2_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jslog2.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jslog2.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jslog2.o.bc -o $OBJ_OUTPUT/jslog2.o -fPIC

extract-bc $OBJ_OUTPUT/jsobj.o
rm $OBJ_OUTPUT/jsobj.o.bc.r
mv $OBJ_OUTPUT/jsobj.o $OBJ_OUTPUT/jsobj_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsobj.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsobj.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsobj.o.bc -o $OBJ_OUTPUT/jsobj.o -fPIC

extract-bc $OBJ_OUTPUT/jslong.o
rm $OBJ_OUTPUT/jslong.o.bc.r
mv $OBJ_OUTPUT/jslong.o $OBJ_OUTPUT/jslong_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jslong.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jslong.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jslong.o.bc -o $OBJ_OUTPUT/jslong.o -fPIC

extract-bc $OBJ_OUTPUT/jsmath.o
rm $OBJ_OUTPUT/jsmath.o.bc.r
mv $OBJ_OUTPUT/jsmath.o $OBJ_OUTPUT/jsmath_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsmath.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsmath.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsmath.o.bc -o $OBJ_OUTPUT/jsmath.o -fPIC

extract-bc $OBJ_OUTPUT/jsnum.o
rm $OBJ_OUTPUT/jsnum.o.bc.r
mv $OBJ_OUTPUT/jsnum.o $OBJ_OUTPUT/jsnum_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsnum.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsnum.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsnum.o.bc -o $OBJ_OUTPUT/jsnum.o -fPIC

extract-bc $OBJ_OUTPUT/jsopcode.o
rm $OBJ_OUTPUT/jsopcode.o.bc.r
mv $OBJ_OUTPUT/jsopcode.o $OBJ_OUTPUT/jsopcode_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsopcode.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsopcode.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsopcode.o.bc -o $OBJ_OUTPUT/jsopcode.o -fPIC

extract-bc $OBJ_OUTPUT/jsparse.o
rm $OBJ_OUTPUT/jsparse.o.bc.r
mv $OBJ_OUTPUT/jsparse.o $OBJ_OUTPUT/jsparse_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsparse.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsparse.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsparse.o.bc -o $OBJ_OUTPUT/jsparse.o -fPIC

extract-bc $OBJ_OUTPUT/jsprf.o
rm $OBJ_OUTPUT/jsprf.o.bc.r
mv $OBJ_OUTPUT/jsprf.o $OBJ_OUTPUT/jsprf_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsprf.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsprf.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsprf.o.bc -o $OBJ_OUTPUT/jsprf.o -fPIC

extract-bc $OBJ_OUTPUT/jsregexp.o
rm $OBJ_OUTPUT/jsregexp.o.bc.r
mv $OBJ_OUTPUT/jsregexp.o $OBJ_OUTPUT/jsregexp_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsregexp.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsregexp.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsregexp.o.bc -o $OBJ_OUTPUT/jsregexp.o -fPIC

extract-bc $OBJ_OUTPUT/jsscan.o
rm $OBJ_OUTPUT/jsscan.o.bc.r
mv $OBJ_OUTPUT/jsscan.o $OBJ_OUTPUT/jsscan_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsscan.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsscan.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsscan.o.bc -o $OBJ_OUTPUT/jsscan.o -fPIC

extract-bc $OBJ_OUTPUT/jsxml.o
rm $OBJ_OUTPUT/jsxml.o.bc.r
mv $OBJ_OUTPUT/jsxml.o $OBJ_OUTPUT/jsxml_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsxml.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsxml.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsxml.o.bc -o $OBJ_OUTPUT/jsxml.o -fPIC

extract-bc $OBJ_OUTPUT/prmjtime.o
rm $OBJ_OUTPUT/prmjtime.o.bc.r
mv $OBJ_OUTPUT/prmjtime.o $OBJ_OUTPUT/prmjtime_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh prmjtime.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.prmjtime.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/prmjtime.o.bc -o $OBJ_OUTPUT/prmjtime.o -fPIC

extract-bc $OBJ_OUTPUT/jsscope.o
rm $OBJ_OUTPUT/jsscope.o.bc.r
mv $OBJ_OUTPUT/jsscope.o $OBJ_OUTPUT/jsscope_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsscope.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsscope.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsscope.o.bc -o $OBJ_OUTPUT/jsscope.o -fPIC

extract-bc $OBJ_OUTPUT/jsscript.o
rm $OBJ_OUTPUT/jsscript.o.bc.r
mv $OBJ_OUTPUT/jsscript.o $OBJ_OUTPUT/jsscript_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsscript.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsscript.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsscript.o.bc -o $OBJ_OUTPUT/jsscript.o -fPIC

extract-bc $OBJ_OUTPUT/jsstr.o
rm $OBJ_OUTPUT/jsstr.o.bc.r
mv $OBJ_OUTPUT/jsstr.o $OBJ_OUTPUT/jsstr_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsstr.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsstr.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsstr.o.bc -o $OBJ_OUTPUT/jsstr.o -fPIC

extract-bc $OBJ_OUTPUT/jsutil.o
rm $OBJ_OUTPUT/jsutil.o.bc.r
mv $OBJ_OUTPUT/jsutil.o $OBJ_OUTPUT/jsutil_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsutil.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsutil.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsutil.o.bc -o $OBJ_OUTPUT/jsutil.o -fPIC

extract-bc $OBJ_OUTPUT/jsxdrapi.o
rm $OBJ_OUTPUT/jsxdrapi.o.bc.r
mv $OBJ_OUTPUT/jsxdrapi.o $OBJ_OUTPUT/jsxdrapi_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsxdrapi.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsxdrapi.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsxdrapi.o.bc -o $OBJ_OUTPUT/jsxdrapi.o -fPIC

extract-bc $OBJ_OUTPUT/jsinvoke.o
rm $OBJ_OUTPUT/jsinvoke.o.bc.r
mv $OBJ_OUTPUT/jsinvoke.o $OBJ_OUTPUT/jsinvoke_backup.o
$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh jsinvoke.o
export Con_PATH=$OBJ_OUTPUT/ConConfig.jsinvoke.o
$ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -c -g $OBJ_OUTPUT/jsinvoke.o.bc -o $OBJ_OUTPUT/jsinvoke.o -fPIC

cd $OBJ_OUTPUT/..
ld -shared  -o Linux_All_DBG.OBJ/libjs.so Linux_All_DBG.OBJ/jsapi.o Linux_All_DBG.OBJ/jsarena.o Linux_All_DBG.OBJ/jsarray.o Linux_All_DBG.OBJ/jsatom.o Linux_All_DBG.OBJ/jsbool.o Linux_All_DBG.OBJ/jscntxt.o Linux_All_DBG.OBJ/jsdate.o Linux_All_DBG.OBJ/jsdbgapi.o Linux_All_DBG.OBJ/jsdhash.o Linux_All_DBG.OBJ/jsdtoa.o Linux_All_DBG.OBJ/jsemit.o Linux_All_DBG.OBJ/jsexn.o Linux_All_DBG.OBJ/jsfun.o Linux_All_DBG.OBJ/jsgc.o Linux_All_DBG.OBJ/jshash.o Linux_All_DBG.OBJ/jsinterp.o Linux_All_DBG.OBJ/jsinvoke.o Linux_All_DBG.OBJ/jsiter.o Linux_All_DBG.OBJ/jslock.o Linux_All_DBG.OBJ/jslog2.o Linux_All_DBG.OBJ/jslong.o Linux_All_DBG.OBJ/jsmath.o Linux_All_DBG.OBJ/jsnum.o Linux_All_DBG.OBJ/jsobj.o Linux_All_DBG.OBJ/jsopcode.o Linux_All_DBG.OBJ/jsparse.o Linux_All_DBG.OBJ/jsprf.o Linux_All_DBG.OBJ/jsregexp.o Linux_All_DBG.OBJ/jsscan.o Linux_All_DBG.OBJ/jsscope.o Linux_All_DBG.OBJ/jsscript.o Linux_All_DBG.OBJ/jsstr.o Linux_All_DBG.OBJ/jsutil.o Linux_All_DBG.OBJ/jsxdrapi.o Linux_All_DBG.OBJ/jsxml.o Linux_All_DBG.OBJ/prmjtime.o  -L$BIN_DIR/install/lib -lplds4 -lplc4 -lnspr4 -lpthread -ldl -lm -L./../../dist/lib -lnspr4

cd $BIN_DIR
extract-bc $BIN_DIR/test-js
rm $BIN_DIR/test-js.bc.r

$ROOT_DIR/tool/staticAnalysis/staticAnalysis.sh test-js
export Con_PATH=$BIN_DIR/ConConfig.test-js
LD_LIBRARY_PATH=$BIN_DIR/install/lib:$LD_LIBRARY_PATH $ROOT_DIR/tool/staticAnalysis/DBDS-INSTRU/dbds-clang-fast++ -g -DXP_UNIX -DJS_THREADSAFE -I $JS_SRC_SRC -I $JS_SRC_SRC/*.OBJ/ $BIN_DIR/test-js.bc -L $JS_SRC_SRC/*.OBJ/ -ljs -o $BIN_DIR/main -pthread
echo "Test Harness Built!"
echo "To Execute, you must specify the shared library path:"
echo "  LD_LIBRARY_PATH=$OBJ_OUTPUT:$BIN_DIR/install/lib $BIN_DIR/main"
