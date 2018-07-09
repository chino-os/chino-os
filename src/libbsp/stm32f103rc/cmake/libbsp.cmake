ADD_DEFINITIONS(-DSTM32F10X_HD -D_RTE_ -D__UVISION_VERSION="523")

INCLUDE_DIRECTORIES(${DRIVER_DIR}/devicetree/hal/inc ${DRIVER_DIR}/devicetree/hal/cmsis/inc ${DRIVER_DIR}/devicetree/hal/rte ${DRIVER_DIR}/devicetree/hal/stdperiph/inc)
