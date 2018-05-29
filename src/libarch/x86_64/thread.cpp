//
// Chino Arch Arch
//
#include "../arch.h"
#include <climits>

#define portINITIAL_RFLAGS	0x206u

extern "C"
{
	void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter)
	{
		auto stack = reinterpret_cast<uint64_t*>(stackPointer);

		context->rdi = parameter;
		context->rflags = portINITIAL_RFLAGS;
		context->rip = entryPoint;

		uint32_t cs;
		__asm volatile("movl %%cs, %0" : "=r" (cs));
		context->cs = cs;

		--stack;
		*--stack = returnAddress;
		context->rsp = uintptr_t(stack);
	}
}