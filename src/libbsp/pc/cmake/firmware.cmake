ADD_CUSTOM_COMMAND(OUTPUT firmware.iso
		COMMAND rm -f firmware.iso
        #COMMAND rm -rf ${FIRMWARE_DIR} && mkdir ${FIRMWARE_DIR} && mkdir ${FIRMWARE_DIR}/BOOT && mkdir ${FIRMWARE_DIR}/BOOT/EFI
        #COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/src/bootloader/BOOTX64.EFI ${FIRMWARE_DIR}/BOOT/EFI
		COMMAND rm -rf ${FIRMWARE_DIR} && mkdir ${FIRMWARE_DIR}
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/src/bootloader/BOOTX64.EFI ${FIRMWARE_DIR}
		COMMAND mkdir ${FIRMWARE_DIR}/chino && mkdir ${FIRMWARE_DIR}/chino/system
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/src/kernel/kernel ${FIRMWARE_DIR}/chino/system
		COMMAND xorriso -as mkisofs -R -f -no-emul-boot -o firmware.iso iso
        DEPENDS bootloader kernel
        COMMENT "Generating firmware.iso ...")
		
ADD_CUSTOM_TARGET(firmware DEPENDS firmware.iso)