﻿cmake_minimum_required (VERSION 3.13)

target_sources(drivers PRIVATE win32-netif.cpp)

CHINOC_RENDER(win32-netif-inl ${CMAKE_CURRENT_BINARY_DIR}/win32-netif.inl ${CMAKE_CURRENT_SOURCE_DIR}/win32-netif.inl.razor)
add_dependencies(drivers win32-netif-inl)
target_include_directories(drivers PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(drivers PRIVATE CONAN_PKG::winpcap)