//
// Kernel Interface
//
extern "C"
{
#include <efi.h>
}

constexpr EFI_MEMORY_TYPE EFI_ChinoKernel_Code = EFI_MEMORY_TYPE(0x80000000 + 1);
constexpr EFI_MEMORY_TYPE EFI_ChinoKernel_Data = EFI_MEMORY_TYPE(0x80000000 + 2);

struct BootParameters
{

};

typedef void(*kernel_entry_t)(const BootParameters& params);