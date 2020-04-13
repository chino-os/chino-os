cmake_minimum_required (VERSION 3.13)

#DTC_COMPILE_DTB(DTB win32.dts)

set(SRCS ../../arch/arm/startup.cpp
         ../../chip/st/stm32f1xx_hd/boot_print.cpp)

target_sources(hal PRIVATE ${SRCS})

#add_dependencies(hal DTB)