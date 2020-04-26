cmake_minimum_required (VERSION 3.13)

set(SRCS usart.cpp
         rcc.cpp
         gpio.cpp)

target_sources(hal PRIVATE ${SRCS})

# Generate kernel lds
CHINOC_RENDER(chino_kernel_lds ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds ${CMAKE_CURRENT_SOURCE_DIR}/../stm32fxxx.lds.razor)