SET(DT_DIR ${DRIVER_DIR}/devicetree)

SET(BSP_SRC ${DT_DIR}/Fdt.cpp
	${DT_DIR}/FdtRoot.cpp)

LIST(APPEND BSP_LIBS fdt)