ADD_CUSTOM_COMMAND(OUTPUT kernel.hex
		COMMAND rm -f kernel.hex
		COMMAND ${CMAKE_ARCH_OBJCOPY} -O ihex ${CMAKE_CURRENT_BINARY_DIR}/src/kernel/kernel kernel.hex
        DEPENDS kernel
        COMMENT "Generating kernel.hex ...")

ADD_CUSTOM_TARGET(firmware DEPENDS kernel.hex)