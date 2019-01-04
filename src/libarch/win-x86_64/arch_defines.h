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
        uintptr_t thread;
        uintptr_t entryPoint;
        uintptr_t parameter;
        uintptr_t returnAddress;
	} ThreadContext_Arch;

#define Port_StackWidth 8
#ifdef __cplusplus
}
#endif
