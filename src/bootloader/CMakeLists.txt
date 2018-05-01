SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${ARCH}-unknown-chino-ld <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
GET_PROPERTY(EFILOAD_INC GLOBAL PROPERTY EFILOAD_INC)
GET_PROPERTY(EFILOAD_SRC GLOBAL PROPERTY EFILOAD_SRC)

GET_PROPERTY(PORTABLE_INC GLOBAL PROPERTY PORTABLE_INC)
GET_PROPERTY(PORTABLE_SRC_ASM GLOBAL PROPERTY PORTABLE_SRC_ASM)

GET_PROPERTY(ACPI_INC GLOBAL PROPERTY ACPI_INC)

# definitions in macros
ADD_DEFINITIONS(-DGNU_EFI_USE_MS_ABI -D__amd64__ -D_ELF -D_ARCH_=${ARCH})

SET(COMMON_FLAGS "\
-fno-stack-protector		 \
-ffreestanding				 \
-fshort-wchar				 \
-fPIC						 \
-mno-red-zone")

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
-nostdlib           \
-znocombreloc       \
-shared				\
-Bsymbolic          \
-T ${GNU_EFI_LIB}/elf_${ARCH}_efi.lds \
-L ${GNU_EFI_LIB}   \
${GNU_EFI_LIB}/crt0-efi-${ARCH}.o")

SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17")
SET(CMAKE_SHARED_LINKER_FLAGS ${LDFLAGS})

# - Add user project to CMake project
MESSAGE("EFILOAD_INC = ${EFILOAD_INC}")
INCLUDE_DIRECTORIES(${GNU_EFI_INC} ${GNU_EFI_INC}/${ARCH} ${EFILOAD_INC} ${PORTABLE_INC} ${ACPI_INC})

FILE(GLOB_RECURSE BOOTLOADER_SRC "*.c" "*.cpp")

SET_PROPERTY(SOURCE ${PORTABLE_SRC_ASM} PROPERTY LANGUAGE C)
SET_SOURCE_FILES_PROPERTIES(${PORTABLE_SRC_ASM} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D__amd64__")
ADD_LIBRARY(bootloader SHARED ${BOOTLOADER_SRC} ${EFILOAD_SRC} ${PORTABLE_SRC_ASM})

TARGET_LINK_LIBRARIES(bootloader efi gnuefi)
SET_TARGET_PROPERTIES(bootloader PROPERTIES 
									SUFFIX .so
									LINKER_LANGUAGE CXX)

# Build target

SET(OBJCOPYFLAGS -j.text -j.sdata -j.data -j.dynamic -j.dynsym  -j.rel -j.rela -j.reloc --target=efi-app-${ARCH})

ADD_CUSTOM_COMMAND(TARGET bootloader POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} ${OBJCOPYFLAGS} $<TARGET_FILE:bootloader> ${CMAKE_CURRENT_BINARY_DIR}/BOOTX64.EFI
		VERBATIM
        DEPENDS bootloader
        COMMENT "Generating .efi file ...")
		
SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_CXX_FLAGS_DEBUG "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")