cmake_minimum_required (VERSION 3.13)

if (CHINO_DRV_WIN32_CONSOLE)
    target_sources(drivers PRIVATE win32-console/win32-console.cpp)
endif()

if (CHINO_DRV_ST_STM32F10X_USART)
    target_sources(drivers PRIVATE st/stm32f10x-usart.cpp)
endif()