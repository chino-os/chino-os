
SET(DT_DIR ${DRIVER_DIR}/devicetree)
SET(SD_DIR ${DRIVER_DIR}/sdio)

SET(BSP_SRC ${DT_DIR}/Fdt.cpp
	${DT_DIR}/FdtRoot.cpp
	
	${SD_DIR}/DummySdioRoot.cpp)

LIST(APPEND BSP_LIBS fdt)