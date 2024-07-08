// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#ifdef WIN32
#include <Windows.h>
#include <isa_availability.h>

extern "C" {
int __isa_available = 0;
int __favor = 0;

#if defined(_M_AMD64)
size_t __memset_nt_threshold = 0x2000000;
size_t __memset_fast_string_threshold = 0x80000;
#endif

int atexit(void(__cdecl *)()) { return 0; }

void abort() {
    while (1)
        ;
}

void guard_check_icall() {}

void _CrtDbgReport() {}

void __cdecl _invalid_parameter(_In_opt_z_ wchar_t const *, _In_opt_z_ wchar_t const *, _In_opt_z_ wchar_t const *,
                                _In_ unsigned int, _In_ uintptr_t) {}

BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

void(__cdecl *__guard_check_icall_fptr)() = guard_check_icall;
void(__cdecl *__guard_dispatch_icall_fptr)() = guard_check_icall;
}

#endif
