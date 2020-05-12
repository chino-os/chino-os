cmake_minimum_required (VERSION 3.13)

target_sources(drivers PRIVATE win32-console.cpp)

CHINOC_RENDER(win32-console-inl ${CMAKE_CURRENT_BINARY_DIR}/win32-console.inl ${CMAKE_CURRENT_SOURCE_DIR}/win32-console.inl.razor)
add_dependencies(drivers win32-console-inl)
target_include_directories(drivers PRIVATE ${CMAKE_CURRENT_BINARY_DIR})