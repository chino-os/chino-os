cmake_minimum_required (VERSION 3.13)

target_sources(chino.drivers PRIVATE host_console.cpp)
target_include_directories(chino.drivers PUBLIC ${WIN_SDK_INCLUDE_DIR})
