cmake_minimum_required (VERSION 3.8)
cmake_policy(SET CMP0079 NEW)

add_subdirectory(crt)

set(SRCS debug.cpp
         processor.cpp)
set(ASM_SRCS arch.asm)
list(TRANSFORM SRCS PREPEND "src/arch/win32/")
list(TRANSFORM ASM_SRCS PREPEND "src/arch/win32/")
target_sources(hal PRIVATE ${SRCS} ${ASM_SRCS})
target_link_libraries(hal PUBLIC crt)