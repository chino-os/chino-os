//
// Chino Arch Arch
//
#include "../arch.h"
#include <climits>
#include <kernel/kdebug.hpp>

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR					( 0x01000000UL )

extern "C"
{
	void SysTick_Handler();

	void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter)
	{
		auto stack = reinterpret_cast<uint32_t*>(stackPointer);

		//  ________
		// |  xPSR  |
		// |   PC   |
		// |   LR   |
		// |   R12  |
		// |   R3   |
		// |   R2   |
		// |   R1   |
		// |   R0   |
		//  --------

		--stack;
		*--stack = portINITIAL_XPSR;
		*--stack = entryPoint;
		*--stack = returnAddress;
		--stack;
		--stack;
		--stack;
		--stack;
		*--stack = parameter;

		context->sp = uintptr_t(stack);
	}
}
