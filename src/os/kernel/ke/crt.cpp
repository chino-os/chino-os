// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#ifdef WIN32
#include <Windows.h>
#include <isa_availability.h>

extern "C" {
int __isa_available = 0;
int __favor = 0;

int atexit(void(__cdecl *)()) { return 0; }

void _CrtDbgReport() {}

void __cdecl _invalid_parameter(_In_opt_z_ wchar_t const *, _In_opt_z_ wchar_t const *, _In_opt_z_ wchar_t const *,
                                _In_ unsigned int, _In_ uintptr_t) {}

BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }
}

#endif
