cmake_minimum_required (VERSION 3.8)

set(BASIC_SRCS src/array.c
	src/dictionary.c
	src/hexdump.c
	src/io.c
	src/lines.c
	src/parser.c
	src/tokenizer.c
	src/variables.c
	arch/chino/arch.c
	arch/chino/main.c
	arch/chino/error.c)

add_library(basic STATIC ${BASIC_SRCS})
target_include_directories(basic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(basic PUBLIC api microrl)