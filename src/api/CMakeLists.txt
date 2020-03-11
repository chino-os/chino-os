cmake_minimum_required (VERSION 3.8)

add_library(api INTERFACE)
target_include_directories(api INTERFACE include)
target_link_libraries(api INTERFACE CONAN_PKG::gsl-lite)