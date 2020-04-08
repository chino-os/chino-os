cmake_minimum_required (VERSION 3.13)

set(SRCS crt.cpp)
if (NOT WIN32)
    add_library(crt STATIC ${SRCS})
    target_link_libraries(crt PUBLIC api)
endif()