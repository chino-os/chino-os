cmake_minimum_required (VERSION 3.13)

include_directories(${CHINO_DDK_INCLUDE_DIR})

add_subdirectory(boot)
add_subdirectory(hal)
add_subdirectory(kernel)
