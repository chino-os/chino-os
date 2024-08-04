cmake_minimum_required(VERSION 3.13)

set(CHINO_ARCH emulator)

add_compile_definitions(-DCHINO_ARCH_EMULATOR=1)
set(WIN_SDK_INCLUDE_DIR ${WIN_SDK_ROOT}/Include/${WIN_SDK_VERSION}/shared ${WIN_SDK_ROOT}/Include/${WIN_SDK_VERSION}/um ${WIN_SDK_ROOT}/Include/${WIN_SDK_VERSION}/winrt)
