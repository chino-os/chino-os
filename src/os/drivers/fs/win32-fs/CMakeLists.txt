cmake_minimum_required (VERSION 3.13)

target_sources(drivers PRIVATE win32-fs.cpp)

CHINOC_RENDER(win32-fs-inl ${CMAKE_CURRENT_BINARY_DIR}/win32-fs.inl ${CMAKE_CURRENT_SOURCE_DIR}/win32-fs.inl.razor)
add_dependencies(drivers win32-fs-inl)
target_include_directories(drivers PRIVATE ${CMAKE_CURRENT_BINARY_DIR})