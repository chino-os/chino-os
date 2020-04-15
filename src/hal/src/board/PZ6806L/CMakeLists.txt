cmake_minimum_required (VERSION 3.13)

DTC_COMPILE_DTB(DTB board.dts)

set(SRCS ../../arch/arm/startup.cpp
         ../../chip/st/stm32f1xx_hd/boot_print.cpp
         dtb.cpp)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR} PROPERTIES
                            OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/board.dtb")

target_sources(hal PRIVATE ${SRCS})
target_link_libraries(hal PRIVATE incbin)
target_compile_definitions(hal PRIVATE -DDTB_PATH="${CMAKE_CURRENT_BINARY_DIR}/board.dtb")

add_dependencies(hal DTB)