//
// Kernel Boot
//
#include "../bsp.hpp"

extern "C" extern void BSPEnterKernel(const BootParameters* params, kernel_entry_t entry);

void Chino::BSPCallKernelEntry(const BootParameters& params, kernel_entry_t entry) noexcept
{
	BSPEnterKernel(&params, entry);
}
