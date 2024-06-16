cmake_minimum_required(VERSION 3.13)

set(SRCS emulator.cpp)
add_executable(chino.emulator ${SRCS})
target_link_libraries(chino.emulator PRIVATE bfg::lyra)

install(TARGETS chino.emulator
        COMPONENT chino.boot
        RUNTIME DESTINATION boot
        )
        