cmake_minimum_required (VERSION 3.13)

set(SRCS uart.cpp)

target_sources(hal PRIVATE ${SRCS})