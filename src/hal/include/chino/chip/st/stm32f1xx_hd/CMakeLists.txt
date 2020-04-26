cmake_minimum_required (VERSION 3.13)

# Generate chipdef.h
CHINOC_RENDER(chino_chip_def ${CMAKE_CURRENT_BINARY_DIR}/chipdef.h ${CMAKE_CURRENT_SOURCE_DIR}/chipdef.h.razor)
target_include_directories(hal PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(hal chino_chip_def)