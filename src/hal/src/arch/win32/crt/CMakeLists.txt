cmake_minimum_required (VERSION 3.8)

set(SRCS rtc.cpp
         crt.def)
add_library(crt SHARED ${SRCS})