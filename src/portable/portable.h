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
	extern void PortSaveThreadContextArch(ThreadContext_Arch* tcontext, InterruptContext_Arch* icontext);
	extern void __attribute__((noreturn)) PortRestoreThreadContextArch(ThreadContext_Arch* tcontext, InterruptContext_Arch* icontext);
	extern void PortSetupSchedulerTimer();
	extern void PortSleepMs(uint32_t ms);

	extern uint8_t PortIOReadUInt8(uint16_t port);
	extern void PortIOWriteUInt8(uint16_t port, uint8_t value);
	extern void PortIOReadUInt32String(uint16_t port, uint32_t* buffer, size_t size);
#ifdef __cplusplus
}
#endif