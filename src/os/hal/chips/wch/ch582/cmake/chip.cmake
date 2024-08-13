cmake_minimum_required(VERSION 3.13)

include(${CHINO_HAL_ROOT}/archs/riscv/riscv32/cmake/arch.cmake)

add_compile_definitions(-DCHINO_CHIP_WCH=1)
add_compile_definitions(-DCHINO_CHIP_CH582=1)
