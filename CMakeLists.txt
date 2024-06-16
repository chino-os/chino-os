cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

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
option(BUILD_EMULATOR "Build emulator" ON)

include(cmake/configure-conan.cmake)
include(cmake/conan.cmake)
include(cmake/default-toolchain.cmake)

if(NOT CONAN_EXPORTED)
    conan_check()
    conan_add_remote(NAME sunnycase URL https://conan.sunnycase.moe INDEX 0)
endif()

if(CONAN_EXPORTED) # in conan local cache
    message(STATUS "Standard Conan Installation")
else() # in user space
    message(STATUS "Auto Cmake Conan Installation")
    include(${CMAKE_SOURCE_DIR}/cmake/conan.cmake)
    conan_cmake_run(CONANFILE conanfile.py
                    BASIC_SETUP
                    OPTIONS ${CONAN_OPTS}
                    SETTINGS ${CONAN_SETTINGS}
                    BUILD missing)
endif()

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
include(cmake/dependencies.cmake)

set(CMAKE_SKIP_RPATH OFF)

set(CHINO_SDK_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/sdk)
set(CHINO_DDK_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/ddk)
set(CHINO_SDK_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/sdk)
set(THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third_party)
set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)
if (APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
else ()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

if (MSVC)
    add_definitions(/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /DUNICODE)
    add_definitions(/D_WIN32_WINNT=_WIN32_WINNT_WIN7)
    add_compile_options(/utf-8 /W3 /WX /EHsc -Wno-unused-function -Wno-unused-const-variable)
    #add_link_options(/NODEFAULTLIB)
    set(PYBIND11_CPP_STANDARD "/std:c++latest")
else()
    add_compile_options(-fvisibility=hidden)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

include_directories(${CHINO_SDK_INCLUDE_DIR})
include_directories(${CHINO_SDK_GENERATED_INCLUDE_DIR})

if (BUILD_EMULATOR)
    add_definitions(-DCHINO_EMULATOR=1)
endif()

add_subdirectory(include/sdk/chino)
add_subdirectory(src)

if(BUILD_TESTING)
endif()
