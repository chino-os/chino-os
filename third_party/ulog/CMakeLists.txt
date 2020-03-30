project(ulog)

set(SRCS ulog.c)

add_library(ulog STATIC ${SRCS})
target_include_directories(ulog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})