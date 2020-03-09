cmake_minimum_required (VERSION 3.8)

set(SRCS startup.cpp)
list(TRANSFORM SRCS PREPEND "src/board/win32/")
target_sources(hal PRIVATE ${SRCS})