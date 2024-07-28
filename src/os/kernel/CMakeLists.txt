cmake_minimum_required(VERSION 3.13)

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

set(SRCS object_pool.cpp)

if (BUILD_EMULATOR)
    add_library(chino.kernel SHARED ${SRCS})
else()
    add_executable(chino.kernel ${SRCS})

    if (APPLE)
        target_link_options(chino.kernel PRIVATE -e _kernel_entry)
    endif()
endif()

add_subdirectory(de)
add_subdirectory(io)
add_subdirectory(ke)
add_subdirectory(mm)
add_subdirectory(ob)
add_subdirectory(ps)

target_link_libraries(chino.kernel PRIVATE chino.hal c cxx_static c.host)

if (WIN32)
    target_link_options(chino.kernel PRIVATE /NODEFAULTLIB)
    target_link_libraries(chino.kernel PRIVATE libvcruntime)
else()
    target_link_options(chino.kernel PRIVATE -nostdlib)
endif()

install(TARGETS chino.kernel
        COMPONENT chino.os
        RUNTIME DESTINATION boot
        LIBRARY DESTINATION boot
        )
        