cmake_minimum_required(VERSION 3.13)

set(SRCS schedule_locks.cpp
         scheduler.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
