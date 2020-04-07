enable_language(ASM_MASM)
set(CHINO_ARCH win32)

set(CHINO_KERNEL_STACK_SIZE "40960")

# Drivers
## serial
set(CHINO_DRV_WIN32_CONSOLE ON)

## fs
set(CHINO_DRV_WIN32_FS ON)

## netif
set(CHINO_DRV_WIN32_NETIF ON)