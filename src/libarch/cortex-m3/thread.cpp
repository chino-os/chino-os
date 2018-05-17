//
// Chino Arch Arch
//
#include "../arch.h"
#include <climits>

extern "C"
{
	void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter)
	{
	}

	void ArchSetupSchedulerTimer()
	{
	}

	void ArchSaveThreadContextArch(ThreadContext_Arch* tcontext, InterruptContext_Arch* icontext)
	{
	}

	void ArchSleepMs(uint32_t ms)
	{
	}
}