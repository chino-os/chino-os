include(${CHINO_ROOT}/src/hal/src/arch/arm/armv7-m/arch.cmake)
set(CHINO_CHIP st/stm32f1xx_hd)

set(CHINO_KERNEL_LDS ${CHINO_ROOT}/src/hal/src/chip/st/stm32f1xx_hd/stm32f103zet6.lds)

set(CHINO_PAGE_SIZE "512")
set(CHINO_KERNEL_STACK_SIZE "2048")
set(CHINO_IDLE_STACK_SIZE "512")

set(CHINO_DRV_ST_STM32F10X_USART ON)