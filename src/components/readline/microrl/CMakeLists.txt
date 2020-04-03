cmake_minimum_required (VERSION 3.8)

set(SRCS src/microrl.c)

add_library(microrl STATIC ${SRCS})
target_include_directories(microrl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(microrl PUBLIC api)