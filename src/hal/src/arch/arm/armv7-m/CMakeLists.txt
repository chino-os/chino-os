cmake_minimum_required (VERSION 3.13)

set(SRCS debug.cpp
         itm.cpp
         processor.cpp)
set(ASM_SRCS ../startup.S
             isr_vector.S)
             
set_property(SOURCE ${ASM_SRCS} PROPERTY LANGUAGE C)
set_source_files_properties(${ASM_SRCS} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D__${CHINO_ARCH}")

target_sources(hal PRIVATE ${SRCS} ${ASM_SRCS})