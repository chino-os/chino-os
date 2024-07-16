cmake_minimum_required (VERSION 3.13)

add_library(chino.drivers STATIC dummy.cpp)
target_link_libraries(chino.drivers PUBLIC c cxx-headers)

include(${CHINO_CONF_GEN_DIR}/chino/conf/drivers.cmake)
