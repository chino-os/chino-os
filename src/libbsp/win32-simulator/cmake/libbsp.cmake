SET(DT_DIR ${DRIVER_DIR}/devicetree)
SET(SIM_DIR ${DRIVER_DIR}/devicetree/simulator)

SET(BSP_SRC ${DT_DIR}/Fdt.cpp
	${DT_DIR}/FdtRoot.cpp
	
	${SIM_DIR}/display/lcd/BasicDisplay.cpp)

LIST(APPEND BSP_LIBS fdt d2d1)