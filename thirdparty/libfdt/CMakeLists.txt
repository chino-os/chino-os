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

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17")
SET(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS}")

# - Add user project to CMake project
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${COMMON_INC} .)

FILE(GLOB_RECURSE FDT_SRC "*.c" "*.cpp")
FILE(GLOB_RECURSE FDT_SRC_ASM "*.S")

SET_PROPERTY(SOURCE ${FDT_SRC_ASM} PROPERTY LANGUAGE C)
SET_SOURCE_FILES_PROPERTIES(${FDT_SRC_ASM} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D${ARCH}")

ADD_LIBRARY(fdt STATIC ${FDT_SRC} ${FDT_SRC_ASM})