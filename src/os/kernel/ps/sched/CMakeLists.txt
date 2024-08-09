cmake_minimum_required(VERSION 3.13)

set(SRCS scheduler.cpp
         synchronizers.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
