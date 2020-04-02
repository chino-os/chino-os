cmake_minimum_required (VERSION 3.8)

add_subdirectory(nr)

add_library(shell STATIC shell.cpp)
target_link_libraries(shell PRIVATE api nr lua basic)