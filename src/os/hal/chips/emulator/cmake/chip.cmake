cmake_minimum_required(VERSION 3.13)

set(CHINO_ARCH emulator)

add_compile_definitions(-DCHINO_ARCH_EMULATOR=1)

if (MSVC)
    if (NOT DEFINED WIN_SDK_INCLUDE_DIR)
        message(FATAL_ERROR "WIN_SDK_INCLUDE_DIR not defined")
    else()
        set(WIN_SDK_INCLUDE_DIR ${WIN_SDK_ROOT}/Include/${WIN_SDK_VERSION}/shared
                                ${WIN_SDK_ROOT}/Include/${WIN_SDK_VERSION}/um
                                ${WIN_SDK_ROOT}/Include/${WIN_SDK_VERSION}/winrt)
    endif()
else()
    if (NOT DEFINED TOOLCHAIN_INCLUDE_DIR)
        message(FATAL_ERROR "TOOLCHAIN_INCLUDE_DIR not defined")
    else()
        add_compile_options("-isystem ${TOOLCHAIN_INCLUDE_DIR}")
    endif()
endif()
