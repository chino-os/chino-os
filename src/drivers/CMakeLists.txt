cmake_minimum_required (VERSION 3.13)

add_library(drivers STATIC)
target_sources(drivers PRIVATE dummy.cpp)
target_link_libraries(drivers PUBLIC api ddk)
target_link_libraries(drivers PRIVATE hal ulog)

if(MSVC)
    target_link_options(drivers PUBLIC "/WHOLEARCHIVE:drivers")
endif()

# Generate drivers inclusion
CHINOC_RENDER_IMM(${CMAKE_CURRENT_BINARY_DIR}/inc_drivers.cmake ${CMAKE_CURRENT_SOURCE_DIR}/inc_drivers.cmake.razor)

add_subdirectory(console/serial-console)
include(${CMAKE_CURRENT_BINARY_DIR}/inc_drivers.cmake)