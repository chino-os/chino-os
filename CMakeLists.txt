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

set(CMAKE_SKIP_RPATH OFF)

set(CHINO_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CHINO_HAL_ROOT ${CMAKE_CURRENT_LIST_DIR}/src/os/hal)
set(CHINO_SDK_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/sdk)
set(CHINO_DDK_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/ddk)
set(CHINO_CONF_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
set(CHINO_SDK_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/sdk)
set(CHINO_THIRD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

include(conf/boards/${CHINO_BOARD}/cmake/board.cmake)
include(src/os/hal/chips/${CHINO_CHIP}/cmake/chip.cmake)
include(cmake/dependencies.cmake)
set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)
if (APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
else ()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

add_compile_definitions(__CHINO__=1)

if (MSVC)
    add_definitions(/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /DUNICODE)
    add_definitions(/D_WIN32_WINNT=_WIN32_WINNT_WIN7)
    add_compile_options(/utf-8 /W3 /WX /wd4273 /guard:cf- -Wno-unused-command-line-argument -Wno-user-defined-literals -Wno-ignored-pragma-intrinsic
                        -Wno-ignored-attributes -Wno-pragma-pack -Wno-nonportable-include-path -Wno-unknown-pragmas -Wno-unused-function
                        -Wno-microsoft-anon-tag -Wno-extern-initializer -Wno-extern-c-compat)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
    add_compile_options(-fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables)
    add_compile_options(-Wall -Wextra -Werror)

    # Disable standard lib.
    add_compile_options(-nostdlib)
    add_link_options(-nostdlib)
    add_compile_options(-isysroot ${CMAKE_CURRENT_BINARY_DIR}/src/crts/picolibc/picolibc/include)
endif()

include_directories(${CHINO_SDK_INCLUDE_DIR})
include_directories(${CHINO_SDK_GENERATED_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/gen)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/crts/picolibc/picolibc/include)

add_subdirectory(include/sdk/chino)

add_subdirectory(src)
#add_subdirectory(third_party/mimalloc)
if (CHINO_EMULATOR)
    set(LLVM_ENABLE_RUNTIMES "libcxx")
    add_subdirectory(third_party/llvm/runtimes)
    set(CHINO_LIBCXX_HEADERS cxx_headers)
    set(CHINO_LIBCXX_STATIC cxx_static)
else()
    set(CHINO_LIBCXX_HEADERS)
    set(CHINO_LIBCXX_STATIC)
endif()

if(BUILD_TESTING)
endif()
