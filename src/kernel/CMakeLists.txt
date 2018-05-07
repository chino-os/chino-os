#SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_LINK} <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

GET_PROPERTY(PORTABLE_INC GLOBAL PROPERTY PORTABLE_INC)
GET_PROPERTY(PORTABLE_SRC_ASM GLOBAL PROPERTY PORTABLE_SRC_ASM)
GET_PROPERTY(PORTABLE_SRC GLOBAL PROPERTY PORTABLE_SRC)

GET_PROPERTY(COMMON_INC GLOBAL PROPERTY COMMON_INC)

GET_PROPERTY(ACPI_INC GLOBAL PROPERTY ACPI_INC)

# definitions in macros
ADD_DEFINITIONS(-DGNU_EFI_USE_MS_ABI -D_ARCH_=${ARCH})

SET(COMMON_FLAGS "\
-Os			\
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
-pie					\
-T ${CMAKE_CURRENT_SOURCE_DIR}/../default.lds \
-Wl,-static")

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17")
SET(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} --specs=nosys.specs")

# - Add user project to CMake project
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${PORTABLE_INC} ${COMMON_INC} ${ACPI_INC})

FILE(GLOB_RECURSE KERNEL_SRC "*.c" "*.cpp")

SET_PROPERTY(SOURCE ${PORTABLE_SRC_ASM} PROPERTY LANGUAGE C)
SET_SOURCE_FILES_PROPERTIES(${PORTABLE_SRC_ASM} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D__amd64__")

ADD_EXECUTABLE(kernel ${KERNEL_SRC} ${PORTABLE_SRC_ASM} ${PORTABLE_SRC})

TARGET_LINK_LIBRARIES(kernel)
SET_TARGET_PROPERTIES(kernel PROPERTIES
							 LINKER_LANGUAGE CXX)
		
SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_CXX_FLAGS_DEBUG "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")