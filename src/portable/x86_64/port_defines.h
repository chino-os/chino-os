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
		uint64_t rax;
		uint64_t rbx;
		uint64_t rcx;
		uint64_t rdx;
		uint64_t rbp;
		uint64_t rdi;
		uint64_t rsi;
		uint64_t rsp;

		uint64_t cs;

		uint64_t rflags;
		uint64_t rip;
	} ThreadContext_Arch;

	typedef struct
	{
		uint64_t rax;
		uint64_t rbx;
		uint64_t rcx;
		uint64_t rdx;
		uint64_t rbp;
		uint64_t rdi;
		uint64_t rsi;
		uint64_t rsp_before;

		uint64_t cs;

		uint64_t rflags;
		uint64_t rip_before;
	} InterruptContext_Arch;
#ifdef __cplusplus
	static_assert(sizeof(InterruptContext_Arch) == 88, "Update size referrenced in the port.");
}
#endif
