cmake_minimum_required (VERSION 3.8)

set(SRCS startup.cpp)

add_library(board OBJECT ${SRCS})
target_link_libraries(board PUBLIC api)