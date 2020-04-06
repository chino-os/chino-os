cmake_minimum_required (VERSION 3.13)

configure_file(include/chino/chino_config.h.in chino_config.h)

add_library(ddk INTERFACE)
target_include_directories(ddk INTERFACE include)
target_include_directories(ddk INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ddk INTERFACE api CONAN_PKG::gsl-lite)