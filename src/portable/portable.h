//
// Chino Arch Port
//
#pragma once
#include "../kernel/kernel_iface.h"
#include <port_defines.h>

#ifdef __cplusplus
extern "C"
{
#endif
	extern void PortEnterKernel(const struct BootParameters* params, kernel_entry_t kernelEntry);

	extern void PortEnableInterrupt();
	extern void PortDisableInterrupt();
	extern void PortHaltProcessor();

	extern void PortInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter);
	extern void PortSetupSchedulerTimer();
#ifdef __cplusplus
}
#endif