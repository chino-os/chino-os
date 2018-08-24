ADD_DEFINITIONS(-DSTM32F10X_HD -D_RTE_ -D__UVISION_VERSION="523")

SET(DT_DIR ${DRIVER_DIR}/devicetree)
SET(ST_DIR ${DT_DIR}/st/stm32f10x)
SET(AT_DIR ${DT_DIR}/atmel)
SET(AD_DIR ${DT_DIR}/adi)
SET(ILI_DIR ${DT_DIR}/ilitek)
SET(GD_DIR ${DT_DIR}/gd)
SET(MC_DIR ${DT_DIR}/microchip)
SET(DM_DIR ${DT_DIR}/davicom)
SET(VS_DIR ${DT_DIR}/vlsi)
SET(ARM_DIR ${DT_DIR}/arm/cortex-m3)

SET(SD_DIR ${DRIVER_DIR}/sdio)
INCLUDE_DIRECTORIES(${ST_DIR}/hal/inc ${ST_DIR}/hal/cmsis/inc ${ST_DIR}/hal/rte ${ST_DIR}/hal/stdperiph/inc)

SET(BSP_SRC ${DT_DIR}/Fdt.cpp
	${DT_DIR}/FdtRoot.cpp
	${ARM_DIR}/Boot.c 
	${ARM_DIR}/threading/Timer.cpp
	${ARM_DIR}/controller/Nvic.cpp
	${ST_DIR}/controller/Nvic.cpp
	${ST_DIR}/diagnostic/Debug.cpp
	${ST_DIR}/controller/Rcc.cpp
	${ST_DIR}/controller/Port.cpp
	${ST_DIR}/controller/Fsmc.cpp
	${ST_DIR}/controller/Dmac.cpp
	${ST_DIR}/io/Gpio.cpp
	${ST_DIR}/io/Usart.cpp
	${ST_DIR}/io/I2c.cpp
	${ST_DIR}/io/Spi.cpp
	${ST_DIR}/io/Sdio.cpp
	${ST_DIR}/display/lcd/lcd8080fsmc.cpp

	${AT_DIR}/storage/eeprom/at24c02.cpp
	${AD_DIR}/sensor/adxl345.cpp
	${ILI_DIR}/display/lcd/ili9486.cpp
	${GD_DIR}/storage/flash/gd25q128.cpp
	${MC_DIR}/network/ethernet/enc28j60.cpp
	${DM_DIR}/network/ethernet/dm9051.cpp
	${VS_DIR}/audio/adapter/vs1053b.cpp
	
	${SD_DIR}/SdioRoot.cpp)

FILE(GLOB_RECURSE ST_HAL_SRC "${ST_DIR}/hal/*.c")
LIST(APPEND BSP_SRC ${ST_HAL_SRC})

SET(DEVICE_TREE_PATH "${BOARD_CMAKE_DIR}/../devicetree/stm32f103rc")
ADD_CUSTOM_COMMAND(OUTPUT devicetree.o
		COMMAND dtc -O dtb -o devicetree.dtb ${DEVICE_TREE_PATH}.dts
		COMMAND ${CMAKE_LINKER} -r -b binary -o devicetree.o devicetree.dtb
        COMMENT "Compiling Device Tree ..."
		DEPENDS ${DEVICE_TREE_PATH}.dts)

LIST(APPEND BSP_SRC devicetree.o)

SET(BSP_SRC_ASM ${ST_DIR}/Boot.S)

LIST(APPEND BSP_LIBS fdt)