project(incbin)

add_library(incbin INTERFACE ${SRCS})
target_include_directories(incbin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})