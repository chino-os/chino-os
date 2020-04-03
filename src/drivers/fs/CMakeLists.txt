cmake_minimum_required (VERSION 3.8)
cmake_policy(SET CMP0079 NEW)

set(SRCS win32-fs/win32-fs.cpp)
list(TRANSFORM SRCS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

target_sources(drivers PRIVATE ${SRCS})