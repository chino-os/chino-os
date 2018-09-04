
SET(DT_DIR ${DRIVER_DIR}/devicetree)
SET(ESP_DIR ${DT_DIR}/espressif/esp32)

SET(SD_DIR ${DRIVER_DIR}/sdio)

SET(BSP_SRC ${DT_DIR}/Fdt.cpp
	${DT_DIR}/FdtRoot.cpp
	${ESP_DIR}/threading/Timer.cpp
	
	${SD_DIR}/DummySdioRoot.cpp)

LIST(APPEND BSP_LIBS fdt)