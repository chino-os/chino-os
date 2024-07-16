cmake_minimum_required(VERSION 3.13)

set(SRCS io_manager.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
target_link_libraries(chino.kernel PRIVATE chino.drivers)
