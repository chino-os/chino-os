#SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_LINK} <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

GET_PROPERTY(PORTABLE_INC GLOBAL PROPERTY PORTABLE_INC)
GET_PROPERTY(PORTABLE_SRC_ASM GLOBAL PROPERTY PORTABLE_SRC_ASM)

# definitions in macros
ADD_DEFINITIONS(-DGNU_EFI_USE_MS_ABI)

SET(COMMON_FLAGS "\
-fno-common \
-ffunction-sections \
-fdata-sections \
-fstrict-volatile-bitfields \
-fno-stack-protector		 \
-fPIC						 \
-ekernel_entry")

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
-nostdlib				\
-static                 \
-pie					\
-Wl,-static             \
-Wl,--start-group       \
-Wl,--whole-archive     \
-Wl,--no-whole-archive  \
-Wl,--end-group         \
-Wl,-EL")

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17")
SET(CMAKE_EXE_LINKER_FLAGS ${LDFLAGS})

# - Add user project to CMake project
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${PORTABLE_INC})

FILE(GLOB_RECURSE KERNEL_SRC "*.c" "*.cpp")

SET_PROPERTY(SOURCE ${PORTABLE_SRC_ASM} PROPERTY LANGUAGE C)
SET_SOURCE_FILES_PROPERTIES(${PORTABLE_SRC_ASM} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D__amd64__")

ADD_EXECUTABLE(kernel ${KERNEL_SRC} ${PORTABLE_SRC_ASM})

TARGET_LINK_LIBRARIES(kernel)
SET_TARGET_PROPERTIES(kernel PROPERTIES 
							 LINKER_LANGUAGE CXX)
		
SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_CXX_FLAGS_DEBUG "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")