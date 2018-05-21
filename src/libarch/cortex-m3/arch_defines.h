//
// Chino Arch Port
//
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	typedef struct
	{
		uint32_t r[13];
		uint32_t sp;
		uint32_t lr;
		uint32_t psr;
		uint32_t pc;
	} ThreadContext_Arch;

	typedef struct
	{
		uint32_t r[13];
		uint32_t sp_before;
		uint32_t lr_before;
		uint32_t psr_before;
		uint32_t pc_before;
	} InterruptContext_Arch;

#define Port_StackWidth 8
#ifdef __cplusplus
	static_assert(sizeof(InterruptContext_Arch) == 68, "Update size referrenced in the port.");
}
#endif
