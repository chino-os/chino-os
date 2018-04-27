//
// Kernel Interface
//
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include <efi.h>
#include <efiapi.h>
#include <stdint.h>
#include <stddef.h>

#define ChinoKernel_StackSize 64u * 1024u
#define EFI_ChinoKernel_Code (EFI_MEMORY_TYPE)(0x80000000 + 1)
#define EFI_ChinoKernel_Data (EFI_MEMORY_TYPE)(0x80000000 + 2)

struct BootParameters
{
	uintptr_t StackPointer;
	EFI_RUNTIME_SERVICES* EfiRuntimeService;
	EFI_MEMORY_DESCRIPTOR* EfiMemoryDescriptor;

	// Gfx
	uintptr_t FrameBufferBase;
	size_t FrameBufferSize;
	size_t FrameBufferWidth;
	size_t FrameBufferHeight;
};

typedef void(*kernel_entry_t)(const struct BootParameters* params);

#ifdef __cplusplus

static_assert(offsetof(BootParameters, StackPointer) == 0);
}
#endif