set(CHINO_ARCH arm/armv7-m)
set(COMP_LINK_OPTIONS -mthumb -march=armv7-m -mfloat-abi=soft -mcpu=cortex-m3 -nostartfiles)
add_compile_options(${COMP_LINK_OPTIONS})
add_link_options(${COMP_LINK_OPTIONS})