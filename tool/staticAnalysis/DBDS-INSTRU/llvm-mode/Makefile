#
# dbds instrumentation - LLVM instrumentation
# -----------------------------------------
#
# LLVM integration design comes wcventure.
#
# Copyright 2020, 2021 wcventure LLC All rights reserved.
#

export Con_PATH = $(PWD)/ConConfig

PREFIX      ?= /usr/local
HELPER_PATH  = $(PREFIX)/lib/afl
BIN_PATH     = $(PREFIX)/bin

VERSION     = $(shell grep '^\#define VERSION ' ./config.h | cut -d '"' -f2)

LLVM_CONFIG ?= llvm-config

CFLAGS      ?= -O3 -funroll-loops
CFLAGS      += -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign \
               -DAFL_PATH=\"$(HELPER_PATH)\" -DBIN_PATH=\"$(BIN_PATH)\" \
               -DVERSION=\"$(VERSION)\" 
ifdef AFL_TRACE_PC
  CFLAGS    += -DUSE_TRACE_PC=1
endif

CXXFLAGS    ?= -O0 -funroll-loops
CXXFLAGS    += -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign \
               -DVERSION=\"$(VERSION)\" -Wno-variadic-macros

# Mark nodelete to work around unload bug in upstream LLVM 5.0+
CLANG_CFL    = `$(LLVM_CONFIG) --cxxflags` -Wl,-znodelete -fno-rtti -fpic $(CXXFLAGS)
CLANG_LFL    = `$(LLVM_CONFIG) --ldflags` $(LDFLAGS)

ifeq "$(shell uname)" "Darwin"
  CLANG_CFL    = `$(LLVM_CONFIG) --cxxflags` -fno-rtti -fpic $(CXXFLAGS)
  CLANG_LFL    = `$(LLVM_CONFIG) --ldflags` $(LDFLAGS)
endif

# User teor2345 reports that this is required to make things work on MacOS X.

ifeq "$(shell uname)" "Darwin"
  CLANG_LFL += -Wl,-flat_namespace -Wl,-undefined,suppress
endif

# We were using llvm-config --bindir to get the location of clang, but
# this seems to be busted on some distros, so using the one in $PATH is
# probably better.

ifeq "$(origin CC)" "default"
  CC         = clang
  CXX        = clang++
endif

ifndef AFL_TRACE_PC
  PROGS      = ../dbds-clang-fast ../dbds-llvm-pass.so ../DBDSFunction.o
else
  PROGS      = ../dbds-clang-fast ../DBDSFunction.o
endif

all: test_deps $(PROGS) test_build all_done

test_deps:
ifndef AFL_TRACE_PC
	@echo "[*] Checking for working 'llvm-config'..."
	@which $(LLVM_CONFIG) >/dev/null 2>&1 || ( echo "[-] Oops, can't find 'llvm-config'. Install clang or set \$$LLVM_CONFIG or \$$PATH beforehand."; echo "    (Sometimes, the binary will be named llvm-config-3.5 or something like that.)"; exit 1 )
else
	@echo "[!] Note: using -fsanitize=trace-pc mode (this will fail with older LLVM)."
endif
	@echo "[*] Checking for working '$(CC)'..."
	@which $(CC) >/dev/null 2>&1 || ( echo "[-] Oops, can't find '$(CC)'. Make sure that it's in your \$$PATH (or set \$$CC and \$$CXX)."; exit 1 )
	@echo "[+] All set and ready to build."

../dbds-clang-fast: dbds-clang-fast.c | test_deps
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	ln -sf dbds-clang-fast ../dbds-clang-fast++

../DBDSFunction.o: DBDSFunction.cpp | test_deps
	$(CXX) -O0 -g $(CXXFLAGS) -fPIC -c $< -o $@
	
../dbds-llvm-pass.so: dbds-llvm-pass.so.cc | test_deps
	$(CXX) $(CLANG_CFL) -shared $< -o $@ $(CLANG_LFL)

test_build: $(PROGS)
	@echo "[*] Testing the CXX wrapper and instrumentation output..."
	unset AFL_USE_ASAN AFL_USE_MSAN AFL_INST_RATIO; AFL_QUIET=1 AFL_PATH=. AFL_CC=$(CC) ../dbds-clang-fast $(CFLAGS) -Wno-return-type -c ./test.c -o test.o $(LDFLAGS) #-fsanitize=address
	$(CXX) ./test.o ../DBDSFunction.o -g -O0 -o test-instr -pthread -ldl #-fsanitize=address
	@echo "[+] All right, the instrumentation seems to be working!"

all_done: test_build
	@echo "[+] All done! You can now use '../dbds-clang-fast' to compile programs."

.NOTPARALLEL: clean

clean:
	rm -f *.o *.so *~ a.out core core.[1-9][0-9]* test-instr .test-instr0 .test-instr1 
	rm -f $(PROGS) ../dbds-clang-fast++
