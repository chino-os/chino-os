cmake_minimum_required (VERSION 3.13)

set(SRCS serial-console/serial-console.cpp)

target_sources(drivers PRIVATE ${SRCS})