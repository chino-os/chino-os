cmake_minimum_required(VERSION 3.13)

set(SRCS dummy.cpp)

if (BUILD_EMULATOR)
    add_library(chino.kernel SHARED ${SRCS})
else()
    add_executable(chino.kernel ${SRCS})

    if (APPLE)
        target_link_options(chino.kernel PRIVATE -e _kernel_entry)
    endif()
endif()

add_subdirectory(de)
add_subdirectory(ke)
add_subdirectory(mm)
add_subdirectory(ps)

if (WIN32)
    target_link_options(chino.kernel PRIVATE /NODEFAULTLIB)
else()
    target_link_options(chino.kernel PRIVATE -nostdlib)
endif()

install(TARGETS chino.kernel
        COMPONENT chino.os
        RUNTIME DESTINATION chino/system
        LIBRARY DESTINATION chino/system
        )
        