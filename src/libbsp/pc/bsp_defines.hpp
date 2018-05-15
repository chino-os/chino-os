//
// Kernel Interface
//
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

#ifdef __cplusplus

static_assert(offsetof(BootParameters, StackPointer) == 0);
}
#endif
