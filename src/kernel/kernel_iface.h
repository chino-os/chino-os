//
// Kernel Interface
//
#pragma once
#ifdef __INTELLISENSE__
#ifndef _ARCH_
#define _ARCH_ x86_64
#define __amd64__ 1
#define _BOARD_ pc
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include <efi.h>
#include <efiapi.h>
#include <acpi/Acpi2_0.h>
#include <stdint.h>
#include <stddef.h>

#define ChinoKernel_StackSize 64u * 1024u
#define EFI_ChinoKernel_Code (EFI_MEMORY_TYPE)(0x80000000 + 1)
#define EFI_ChinoKernel_Data (EFI_MEMORY_TYPE)(0x80000000 + 2)

struct BootParameters
{
	uintptr_t StackPointer;

	// EFI
	struct
	{
		EFI_RUNTIME_SERVICES* RuntimeService;
		EFI_MEMORY_DESCRIPTOR* MemoryDescriptor;
		size_t MemoryDescriptorSize, MemoryDescriptorCount;
	} Efi;

	// Gfx
	struct
	{
		uintptr_t Base;
		size_t Size;
		size_t Width;
		size_t Height;
	} FrameBuffer;

	// ACPI

	struct
	{
		EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* Rsdp;
	} Acpi;
};

typedef void(*kernel_entry_t)(const struct BootParameters* params);

void Kernel_OnTimerHandler(void* interruptContext);

#ifdef __cplusplus

static_assert(offsetof(BootParameters, StackPointer) == 0);
}
#endif

#ifndef MAKEPATH

#define IDENT(x) x
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define MAKEPATH(x,y) STR(IDENT(x)IDENT(y))
#define MAKEPATH3(x,y,z) STR(IDENT(x)IDENT(y)IDENT(z))

#endif
