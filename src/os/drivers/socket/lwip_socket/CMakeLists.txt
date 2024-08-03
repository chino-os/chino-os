cmake_minimum_required (VERSION 3.13)

add_subdirectory(lwip/contrib/ports/chino/lib)

target_sources(chino.drivers PRIVATE lwip_socket.cpp)
target_link_libraries(chino.drivers PRIVATE lwip)
