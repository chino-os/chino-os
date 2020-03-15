cmake_minimum_required (VERSION 3.8)

configure_file(include/chino/chino_config.h.in chino_config.h)

add_library(api INTERFACE)
target_include_directories(api INTERFACE include)
target_include_directories(api INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(api INTERFACE CONAN_PKG::gsl-lite)