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
	extern void ArchEnableInterrupt();
	extern void ArchDisableInterrupt();
	extern void ArchHaltProcessor();

	extern void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter);
	extern bool ArchValidateThreadContext(ThreadContext_Arch* context, uintptr_t stackTop, uintptr_t stackBottom);
#ifdef __cplusplus
}
#endif
