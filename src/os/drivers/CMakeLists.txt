cmake_minimum_required (VERSION 3.13)

add_library(drivers STATIC dummy.cpp)

include(${CHINO_CONF_GEN_DIR}/chino/conf/drivers.cmake)
