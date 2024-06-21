cmake_minimum_required(VERSION 3.13)

set(SRCS crt.cpp
         startup.cpp)

target_sources(chino.kernel PRIVATE ${SRCS})
