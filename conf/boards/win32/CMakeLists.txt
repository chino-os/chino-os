cmake_minimum_required (VERSION 3.13)

set(RES Resource.rc)
set(SRCS startup.cpp)

target_sources(hal PRIVATE ${SRCS})

add_library(winres OBJECT ${RES})
target_include_directories(winres PUBLIC ${CMAKE_CURRENT_BINARY_DIR})