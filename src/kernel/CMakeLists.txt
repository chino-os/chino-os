INCLUDE(${BOARD_CMAKE_DIR}/kernel.cmake)

SET(COMMON_FLAGS "\
-Os			\
-fno-common \
-fexceptions \
-ffunction-sections \
-fdata-sections \
-fstrict-volatile-bitfields \
-fno-stack-protector		 \
-fPIC						 \
-eBSPKernelEntry")

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
-T ${BOARD_LDS_DIR}/kernel.lds \
-Wl,-static")

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17")
SET(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} ${BOARD_LDFLAGS}")

# - Add user project to CMake project
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${COMMON_INC})

FILE(GLOB_RECURSE KERNEL_SRC "*.c" "*.cpp")

ADD_EXECUTABLE(kernel ${KERNEL_SRC})

TARGET_LINK_LIBRARIES(kernel arch bsp)
SET_TARGET_PROPERTIES(kernel PROPERTIES
							LINKER_LANGUAGE CXX)
