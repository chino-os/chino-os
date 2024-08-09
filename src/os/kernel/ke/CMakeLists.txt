cmake_minimum_required(VERSION 3.13)

set(SRCS ke_services.cpp
         startup.cpp)

target_sources(chino.kernel PRIVATE ${SRCS})

if (MSVC)
    target_compile_options(chino.kernel PRIVATE -Wno-inconsistent-dllimport)
endif()
