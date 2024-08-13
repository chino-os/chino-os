cmake_minimum_required(VERSION 3.13)

if(WIN32)
set(SRCS pe/pe_loader.cpp
)
else()
set(SRCS)
endif()

target_sources(chino.kernel PRIVATE ${SRCS})
