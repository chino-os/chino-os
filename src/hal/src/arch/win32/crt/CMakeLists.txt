cmake_minimum_required (VERSION 3.13)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(SRCS crt.cpp
             crtd.def)
else()
    set(SRCS crt.cpp
             crt.def)
endif()
add_library(crt SHARED ${SRCS})
target_include_directories(crt PRIVATE inc)
target_compile_definitions(crt PRIVATE -DCRTDLL2 -D_CRTBLD)