//
// Chino Arch Port
//
#pragma once
#include "../kernel/kernel_iface.h"

#ifdef __cplusplus
extern "C"
{
#endif
	extern void PortEnterKernel(const struct BootParameters* params, kernel_entry_t kernelEntry);
#ifdef __cplusplus
}
#endif