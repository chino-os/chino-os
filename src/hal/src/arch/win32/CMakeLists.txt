cmake_minimum_required (VERSION 3.8)

set(SRCS debug.cpp
         processor.cpp)
list(TRANSFORM SRCS PREPEND "src/arch/win32/")
target_sources(hal PRIVATE ${SRCS})