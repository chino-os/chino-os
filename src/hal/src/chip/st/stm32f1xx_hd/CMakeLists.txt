cmake_minimum_required (VERSION 3.13)

set(SRCS usart.cpp
         rcc.cpp
         gpio.cpp)

target_sources(hal PRIVATE ${SRCS})