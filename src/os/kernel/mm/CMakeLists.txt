cmake_minimum_required(VERSION 3.13)

set(SRCS free_page_list.cpp
         memory_manager.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
