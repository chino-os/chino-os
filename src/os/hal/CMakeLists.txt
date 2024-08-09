cmake_minimum_required(VERSION 3.13)

add_library(chino.hal STATIC)
target_link_libraries(chino.hal PUBLIC c cxx-headers)

add_subdirectory(archs/${CHINO_ARCH})
add_subdirectory(chips/${CHINO_CHIP})
