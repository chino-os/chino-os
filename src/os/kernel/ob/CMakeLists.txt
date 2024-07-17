cmake_minimum_required(VERSION 3.13)

set(SRCS directory.cpp
         file.cpp
         handle_table.cpp
         object.cpp
         object_manager.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
