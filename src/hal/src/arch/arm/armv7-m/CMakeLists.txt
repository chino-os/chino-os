cmake_minimum_required (VERSION 3.13)

set(SRCS debug.cpp)
#set(ASM_SRCS arch.asm)
target_sources(hal PRIVATE ${SRCS} ${ASM_SRCS})