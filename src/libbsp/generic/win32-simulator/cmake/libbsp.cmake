SET(DT_DIR ${DRIVER_DIR}/devicetree)
SET(SIM_DIR ${DRIVER_DIR}/devicetree/simulator)
SET(SD_DIR ${DRIVER_DIR}/sdio)

SET(BSP_SRC ${DT_DIR}/Fdt.cpp
	${DT_DIR}/FdtRoot.cpp
	
	${SIM_DIR}/display/lcd/BasicDisplay.cpp
	${SIM_DIR}/storage/SDStorage.cpp

	${SD_DIR}/DummySdioRoot.cpp)

LIST(APPEND BSP_LIBS fdt ddraw dxguid vhdf)