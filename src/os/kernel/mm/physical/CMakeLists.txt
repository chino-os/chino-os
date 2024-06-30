cmake_minimum_required(VERSION 3.13)

set(SRCS physical_allocator.cpp
         physical_segment.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
