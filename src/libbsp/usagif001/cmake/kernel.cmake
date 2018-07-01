SET(BOARD_C_FLAGS "-fno-pie")
SET(BOARD_CXX_FLAGS "-fno-pie")
SET(BOARD_LDFLAGS "-Wl,-z,relro,-z,now -Wl,--no-dynamic-linker")
SET(NO_CRTBEGIN TRUE)