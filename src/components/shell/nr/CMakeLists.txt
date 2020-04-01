cmake_minimum_required (VERSION 3.8)

set(NR_SRCS src/ansi.c
	src/ansi_port.c
	src/nr_micro_shell.c
	examples/nr_micro_shell_commands.c)

add_library(nr STATIC ${NR_SRCS})
target_include_directories(nr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(nr PUBLIC api)