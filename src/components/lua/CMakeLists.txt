cmake_minimum_required (VERSION 3.8)

set(LUA_SRCS lua/lapi.c
	lua/lauxlib.c
	lua/lbaselib.c
	lua/lbitlib.c
	lua/lcode.c
	lua/lcorolib.c
	lua/lctype.c
	lua/ldblib.c
	lua/ldebug.c
	lua/ldo.c
	lua/ldump.c
	lua/lfunc.c
	lua/lgc.c
	lua/llex.c
	lua/liolib.c
	lua/lmathlib.c
	lua/lmem.c
	lua/loadlib.c
	lua/lobject.c
	lua/lopcodes.c
	lua/loslib.c
	lua/lparser.c
	lua/lstate.c
	lua/lstring.c
	lua/lstrlib.c
	lua/ltable.c
	lua/ltablib.c
	lua/ltm.c
	lua/lua.c
	lua/lundump.c
	lua/lutf8lib.c
	lua/lvm.c
	lua/lzio.c)
set(SRCS)

add_library(lua STATIC ${LUA_SRCS} ${SRCS})
target_include_directories(lua PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#target_include_directories(lua PRIVATE ports/chino)
target_link_libraries(lua PUBLIC api)