cmake_minimum_required(VERSION 3.13)

set(SRCS arch.cpp
         emulator_cpu.cpp
         system_tick.cpp
)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(amd64)|(AMD64)|(x86_64)|(X86_64)")
    list(APPEND SRCS x86_64.S)
endif()

target_sources(chino.kernel PRIVATE ${SRCS})
