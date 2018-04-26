//
// Kernel Entry
//
#include "kernel_iface.h"

extern "C" void kernel_entry(const BootParameters* params)
{
	char str[] = { 0x53, 0x53, 0x44, 0x54, 0 };
	//params->EfiRuntimeService->ResetSystem(EFI_RESET_TYPE::EfiResetWarm, EFI_SUCCESS, 0, nullptr);
	while (1);
}