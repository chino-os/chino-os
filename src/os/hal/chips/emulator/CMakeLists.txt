cmake_minimum_required(VERSION 3.13)

set(SRCS startup.cpp
)

target_sources(chino.hal PRIVATE ${SRCS})
