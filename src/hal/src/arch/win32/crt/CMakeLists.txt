cmake_minimum_required (VERSION 3.8)

set(SRCS crt.cpp
         crt.def)
add_library(crt SHARED ${SRCS})
target_include_directories(crt PRIVATE inc)
target_compile_definitions(crt PRIVATE -DCRTDLL2 -D_CRTBLD)