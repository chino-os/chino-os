cmake_minimum_required(VERSION 3.13)

set(CHINO_ARCH emulator)

add_compile_definitions(-DCHINO_ARCH_EMULATOR=1)

if (NOT DEFINED ENV{WindowsSdkDir})
    message(FATAL_ERROR "Enviroment WindowsSdkDir is not defined")
endif()

if (NOT DEFINED WIN_SDK_INCLUDE_ROOT)
    set(WIN_SDK_INCLUDE_ROOT $ENV{WindowsSdkDir}Include/$ENV{WindowsSdkVersion})
    message(STATUS "Fvck")
endif()

set(WIN_SDK_INCLUDE_DIR ${WIN_SDK_INCLUDE_ROOT}/shared
                        ${WIN_SDK_INCLUDE_ROOT}/um
                        ${WIN_SDK_INCLUDE_ROOT}/winrt)
