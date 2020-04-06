cmake_minimum_required (VERSION 3.13)

set(SRCS simple-bus/simple-bus.cpp)

target_sources(drivers PRIVATE ${SRCS})