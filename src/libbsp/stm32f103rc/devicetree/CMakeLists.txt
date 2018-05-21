
ADD_CUSTOM_COMMAND(OUTPUT stm32f103rc.dtb
		COMMAND dtc -O dtb -o ${CMAKE_CURRENT_SOURCE_DIR}/stm32f103rc.dtb ${CMAKE_CURRENT_SOURCE_DIR}/stm32f103rc.dts
        COMMENT "Compiling Device Tree ...")

ADD_CUSTOM_TARGET(devicetree DEPENDS stm32f103rc.dtb)