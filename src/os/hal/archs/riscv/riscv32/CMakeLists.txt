cmake_minimum_required(VERSION 3.13)

set(SRCS arch.cpp
)

target_sources(chino.hal PRIVATE ${SRCS})
