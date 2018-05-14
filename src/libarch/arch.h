//
// Chino Arch Arch
//
#pragma once
#include <kernel/kernel_iface.h>
#include MAKEPATH(_ARCH_/,arch_defines.h)

#ifdef __cplusplus
extern "C"
{
#endif
	extern void ArchEnterKernel(const struct BootParameters* params, kernel_entry_t kernelEntry);

	extern void ArchEnableInterrupt();
	extern void ArchDisableInterrupt();
	extern void ArchHaltProcessor();

	extern void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter);
	extern void ArchSaveThreadContextArch(ThreadContext_Arch* tcontext, InterruptContext_Arch* icontext);
	extern void __attribute__((noreturn)) ArchRestoreThreadContextArch(ThreadContext_Arch* tcontext, InterruptContext_Arch* icontext);
	extern void ArchSetupSchedulerTimer();
	extern void ArchSleepMs(uint32_t ms);

	extern uint8_t ArchIOReadUInt8(uint16_t Arch);
	extern void ArchIOWriteUInt8(uint16_t Arch, uint8_t value);
	extern void ArchIOReadUInt32String(uint16_t Arch, uint32_t* buffer, size_t size);
#ifdef __cplusplus
}
#endif
