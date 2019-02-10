cmake_minimum_required (VERSION 2.6)
project (RV_test C)

set (RV_VERSION_MAJOR 0)
set (RV_VERSION_MINOR 1)

option(RV32I "Use Risc-V32I" ON)
option(RV32E "Use Risc-V32E" OFF)
configure_file (
  "${PROJECT_SOURCE_DIR}/rvConfig.h.in"
  "${PROJECT_BINARY_DIR}/rvConfig.h"
  )
include_directories(${PROJECT_SOURCE_DIR}/3rd)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

set(SRC_LIST rv_core.c 
             rv_mem.c 
             testAll.c 
    )
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(riscV_test ${SRC_LIST})
