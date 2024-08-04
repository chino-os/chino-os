cmake_minimum_required(VERSION 3.15)

set(SRCS ble_manager.cpp
         socket_manager.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
target_link_libraries(chino.kernel PRIVATE runtimeobject)
