INCLUDE(${BOARD_CMAKE_DIR}/libbsp.cmake)

SET(COMMON_FLAGS "\
-Os			\
-fno-common \
-fexceptions \
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

SET(LDFLAGS "\
-nostartfiles           \
-Wl,-static")

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} ${BOARD_C_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} ${BOARD_CXX_FLAGS} -std=gnu++17")
SET(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} ${BOARD_LDFLAGS}")

# - Add user project to CMake project
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${COMMON_INC})

FILE(GLOB_RECURSE BSP_SRC "${BOARD}/*.c" "${BOARD}/*.cpp")
FILE(GLOB_RECURSE BSP_SRC_ASM "${BOARD}/*.S")

SET_PROPERTY(SOURCE ${BSP_SRC_ASM} PROPERTY LANGUAGE C)
SET_SOURCE_FILES_PROPERTIES(${BSP_SRC_ASM} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D${ARCH}")

ADD_LIBRARY(bsp STATIC ${BSP_SRC} ${BSP_SRC_ASM})

TARGET_LINK_LIBRARIES(bsp ${BSP_LIBS})
SET_TARGET_PROPERTIES(bsp PROPERTIES
							LINKER_LANGUAGE CXX)
							
IF(${BSP_DEPENDENCIES})
	ADD_DEPENDENCIES(bsp ${BSP_DEPENDENCIES})
ENDIF()
