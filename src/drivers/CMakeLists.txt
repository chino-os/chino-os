cmake_minimum_required (VERSION 3.8)

add_library(drivers STATIC)
target_sources(drivers PRIVATE dummy.cpp)
target_link_libraries(drivers PUBLIC api ddk)
target_link_libraries(drivers PRIVATE hal)

if(MSVC)
    target_link_options(drivers PUBLIC "/WHOLEARCHIVE:drivers")
endif()

add_subdirectory(bus)
add_subdirectory(serial)