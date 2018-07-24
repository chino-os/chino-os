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

SET(COMMON_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
SET(COMMON_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17")
SET(COMMON_ASM_FLAGS "-x assembler-with-cpp -D__${ARCH}")

EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtbegin.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTBEGIN_OBJ)
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtend.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTEND_OBJ)
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crti.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTI_OBJ)
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtn.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTN_OBJ)

IF(NO_CRTBEGIN)
	set(CMAKE_CXX_LINK_EXECUTABLE
	"<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRTI_OBJ} <OBJECTS> ${CRTN_OBJ} -o <TARGET> <LINK_LIBRARIES>")
ELSE()
	set(CMAKE_CXX_LINK_EXECUTABLE
	"<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRTI_OBJ} ${CRTBEGIN_OBJ} <OBJECTS> ${CRTEND_OBJ} ${CRTN_OBJ} -o <TARGET> <LINK_LIBRARIES>")
ENDIF()