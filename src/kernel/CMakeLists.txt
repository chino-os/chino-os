INCLUDE(${BOARD_CMAKE_DIR}/kernel.cmake)

SET(COMMON_FLAGS "\
-Os			\
-fno-common \
-ffunction-sections \
-fdata-sections \
-fstrict-volatile-bitfields \
-fno-stack-protector")

SET(COMMON_WARNING_FLAGS "-Wall \
-Werror=all \
-Wno-error=unused-function \
-Wno-error=unused-but-set-variable \
-Wno-error=unused-variable \
-Wno-error=deprecated-declarations \
-Wextra \
-Werror=frame-larger-than=65536 \
-Wno-unused-parameter \
-Wno-sign-compare \
-Wno-old-style-declaration")

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} ${BOARD_C_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} ${BOARD_CXX_FLAGS} -std=gnu++17")

EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtbegin.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTBEGIN_OBJ)
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtend.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTEND_OBJ)
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crti.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTI_OBJ)
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtn.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTN_OBJ)

set(CMAKE_CXX_LINK_EXECUTABLE
"<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRTI_OBJ} ${CRTBEGIN_OBJ} <OBJECTS> ${CRTEND_OBJ} ${CRTN_OBJ} -o <TARGET> <LINK_LIBRARIES>")

SET(LDFLAGS "\
-nostartfiles           \
-T ${BOARD_LDS_DIR}/kernel.lds \
-Wl,-static")

SET(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} ${BOARD_LDFLAGS}")

# - Add user project to CMake project
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${COMMON_INC})

FILE(GLOB_RECURSE KERNEL_SRC "*.c" "*.cpp")

ADD_EXECUTABLE(kernel ${KERNEL_SRC})

TARGET_LINK_LIBRARIES(kernel arch bsp)
SET_TARGET_PROPERTIES(kernel PROPERTIES
							LINKER_LANGUAGE CXX)
