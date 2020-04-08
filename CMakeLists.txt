cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project("chino")

include(cmake/conan.cmake)

conan_check()
conan_add_remote(NAME bincrafts URL https://api.bintray.com/conan/bincrafters/public-conan)
if (APPLE)
    conan_add_remote(NAME sunnycase URL https://conan.sunnycase.moe)
else()
    conan_add_remote(NAME sunnycase INDEX 0 URL https://conan.sunnycase.moe)
endif()

if (NOT CHINO_BOARD)
    message(FATAL "Please specify CHINO_BOARD")
else()
    add_definitions(-DCHINO_BOARD=${CHINO_BOARD})
    add_definitions(-DCHINO_BOARD_HEADER=<chino/board/${CHINO_BOARD}/board.h>)
endif()

if ("${CHINO_BOARD}" STREQUAL "win32")
    conan_cmake_run(CONANFILE conanfile-win32.txt
                    BASIC_SETUP CMAKE_TARGETS
                    SETTINGS compiler.cppstd=20
                    BUILD missing)
else()
    conan_cmake_run(CONANFILE conanfile.txt
                    BASIC_SETUP CMAKE_TARGETS
                    SETTINGS compiler.cppstd=20
                    BUILD missing)
endif()

set(THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third_party)
set(TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR}/tools)
 
if (MSVC)
    enable_language(ASM_MASM)
    add_definitions(/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /DNOMINMAX /DUNICODE /D_UNICODE)
    add_compile_options(/Zc:threadSafeInit- /utf-8 /wd4200 /Oi)
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
else()
    enable_language(ASM)
    add_compile_options(-Wno-multichar -fno-common -ffunction-sections -fdata-sections)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
endif()

set(CHINO_ROOT ${CMAKE_CURRENT_LIST_DIR})
include(cmake/toolchain.cmake)
include(cmake/config.cmake)
include(cmake/dtc.cmake)
include(cmake/boards/${CHINO_BOARD}.cmake)
include(cmake/definitions.cmake)

add_subdirectory(src/api)
add_subdirectory(src/ddk)
add_subdirectory(src/hal)
add_subdirectory(src/kernel)
add_subdirectory(src/components)
add_subdirectory(src/drivers)
add_subdirectory(third_party/libfdt)
add_subdirectory(third_party/ulog)
add_subdirectory(third_party/lwip)