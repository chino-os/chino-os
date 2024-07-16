cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED CHINO_VERSION)
    set(CHINO_VERSION "0.1.0")
endif()

if(DEFINED ENV{CHINO_VERSION_SUFFIX})
    set(CHINO_VERSION_SUFFIX $ENV{CHINO_VERSION_SUFFIX})
endif()

if(NOT DEFINED CHINO_VERSION_SUFFIX)
    find_package (Git)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESC
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CHINO_VERSION_SUFFIX "-${GIT_DESC}")
endif()

if (NOT PACKAGE_VERSION)
    set(PACKAGE_VERSION
        "${CHINO_VERSION}${CHINO_VERSION_SUFFIX}")
endif()

project(CHINO
  VERSION ${CHINO_VERSION}
  LANGUAGES C CXX ASM)

option(BUILD_TESTING "Build test programs" OFF)
option(CHINO_BOARD "Board name" "emulator")

include(cmake/configure-conan.cmake)
include(cmake/conan.cmake)
include(cmake/default-toolchain.cmake)

include(conf/boards/${CHINO_BOARD}/cmake/board.cmake)
include(src/os/hal/chips/${CHINO_CHIP}/cmake/chip.cmake)
include(cmake/dependencies.cmake)

set(CMAKE_SKIP_RPATH OFF)

set(CHINO_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CHINO_SDK_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/sdk)
set(CHINO_DDK_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/ddk)
set(CHINO_CONF_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
set(CHINO_SDK_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/sdk)
set(CHINO_THIRD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)
set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)
if (APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
else ()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

add_compile_definitions(_CHINO=1)

if (MSVC)
    add_definitions(/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /DUNICODE)
    add_definitions(/D_WIN32_WINNT=_WIN32_WINNT_WIN7)
    add_compile_options(/utf-8 /W3 /WX /wd4273 /guard:cf- -Wno-unused-command-line-argument -Wno-user-defined-literals -Wno-ignored-pragma-intrinsic
                        -Wno-ignored-attributes -Wno-pragma-pack -Wno-nonportable-include-path -Wno-unknown-pragmas -Wno-unused-function)
    add_link_options(/NODEFAULTLIB /GUARD:NO)
    # Disable C++ exceptions.
    string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c- /GS-")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS-")
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    add_definitions(-D_HAS_EXCEPTIONS=0)

    # Disable RTTI.
    string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    set(PYBIND11_CPP_STANDARD "/std:c++latest")

    # Disable standard lib.
    add_compile_options(/X)
    add_compile_definitions(__STDC__=1)
else()
    # Disable C++ exceptions.
    add_compile_options(-fno-exceptions -fno-rtti -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables)
    add_compile_options(-fvisibility=hidden)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

include_directories(${CHINO_SDK_INCLUDE_DIR})
include_directories(${CHINO_SDK_GENERATED_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/gen)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/crts/picolibc/picolibc/include)

add_subdirectory(include/sdk/chino)

add_subdirectory(src)
#add_subdirectory(third_party/mimalloc)
set(LLVM_ENABLE_RUNTIMES "libcxx")
add_subdirectory(third_party/llvm/runtimes)


if(BUILD_TESTING)
endif()
