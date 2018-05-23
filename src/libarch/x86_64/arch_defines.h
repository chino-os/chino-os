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

		uint64_t r8;
		uint64_t r9;
		uint64_t r10;
		uint64_t r11;
		uint64_t r12;
		uint64_t r13;
		uint64_t r14;
		uint64_t r15;

		uint64_t rip;
		uint64_t cs;
		uint64_t rflags;
		uint64_t rsp;
	} ThreadContext_Arch;

	extern uint8_t ArchIOReadUInt8(uint16_t Arch);
	extern void ArchIOWriteUInt8(uint16_t Arch, uint8_t value);
	extern void ArchIOReadUInt32String(uint16_t Arch, uint32_t* buffer, size_t size);

#define Port_StackWidth 8
#ifdef __cplusplus
}
#endif