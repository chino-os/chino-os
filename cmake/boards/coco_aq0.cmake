include(${CHINO_ROOT}/src/hal/src/arch/arm/armv7-m/arch.cmake)
set(CHINO_CHIP wm/w600)

set(CHINO_KERNEL_LDS ${CHINO_ROOT}/src/hal/src/chip/wm/w600/w600.lds)

set(CHINO_KERNEL_STACK_SIZE "8192")