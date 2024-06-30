cmake_minimum_required(VERSION 3.13)

set(SRCS emulator.cpp)
add_executable(chino.emulator ${SRCS})
target_link_libraries(chino.emulator PRIVATE bfg::lyra)

if (NOT MSVC)
    target_compile_options(chino.emulator PRIVATE -fexceptions)
    if (APPLE)
        target_link_libraries(chino.emulator PRIVATE -stdlib=libc++)
    endif()
endif()

install(TARGETS chino.emulator
        COMPONENT chino.boot
        RUNTIME DESTINATION boot
        )
        