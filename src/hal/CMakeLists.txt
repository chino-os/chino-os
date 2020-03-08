cmake_minimum_required (VERSION 3.8)
add_subdirectory(src/arch/${CHINO_ARCH})
add_subdirectory(src/board/${CHINO_BOARD})

set(SRCS src/arch/dummy.cpp)

add_library(hal STATIC ${SRCS})
target_include_directories(hal PUBLIC include)
target_link_libraries(hal PRIVATE arch board)