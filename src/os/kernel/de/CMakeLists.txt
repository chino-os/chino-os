cmake_minimum_required(VERSION 3.13)

set(SRCS fail_fast.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
