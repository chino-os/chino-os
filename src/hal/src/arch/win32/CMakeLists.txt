cmake_minimum_required (VERSION 3.13)

add_subdirectory(crt)

set(SRCS debug.cpp
         processor.cpp)
set(ASM_SRCS arch.asm)
target_sources(hal PRIVATE ${SRCS} ${ASM_SRCS})
target_link_libraries(hal PUBLIC crt)