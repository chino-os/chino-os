cmake_minimum_required(VERSION 3.13)

set(SRCS process.cpp
         thread.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
