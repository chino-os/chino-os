set(CHINO_ARCH arm/armv7-m)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -specs=nosys.specs -march=armv7-m -mcpu=cortex-m3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb -specs=nosys.specs -march=armv7-m -mcpu=cortex-m3")

set(CHINO_KERNEL_STACK_SIZE "8192")