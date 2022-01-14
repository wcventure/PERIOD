project(IncreFuzz-llvm-pass)
cmake_minimum_required(VERSION 3.5)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(SensitiveOperationsPass)
add_subdirectory(DryRunPass)
add_subdirectory(DBDSPass)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fpic -O3 -funroll-loops -Wall -D_FORTIFY_SOURCE=2 -g -Wno-vla-extension -Wno-unknown-warning-option -Wno-pointer-sign -Wno-variadic-macros -shared  -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fpic -O3 -funroll-loops -Wall -D_FORTIFY_SOURCE=2 -g -Wno-vla-extension -Wno-unknown-warning-option -Wno-pointer-sign -Wno-variadic-macros -shared  -fPIC")

set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-flat_namespace -Wl,-undefined,suppress")