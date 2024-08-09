cmake_minimum_required(VERSION 3.13)

set(SRCS cpu_physical_allocator.cpp
         physical_allocator.cpp
         physical_segment.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
