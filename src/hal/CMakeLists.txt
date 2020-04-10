cmake_minimum_required (VERSION 3.8)

add_library(hal STATIC)
target_sources(hal PRIVATE src/arch/dummy.cpp)
target_include_directories(hal PUBLIC include)
target_include_directories(hal PUBLIC include/chino/board/${CHINO_BOARD})
target_link_libraries(hal PUBLIC api ddk)

add_subdirectory(src/arch/${CHINO_ARCH})
add_subdirectory(src/chip/${CHINO_CHIP})
add_subdirectory(src/board/${CHINO_BOARD})