cmake_minimum_required (VERSION 3.8)
cmake_policy(SET CMP0079 NEW)

DTC_COMPILE_DTB(DTB win32.dts)

set(RES Resource.rc)
set(SRCS startup.cpp
         dtb.cpp)
list(TRANSFORM RES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM SRCS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

target_sources(hal PRIVATE ${SRCS})

add_library(winres OBJECT ${RES})
target_include_directories(winres PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(hal DTB)