// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator_cpu.h"
#include <chino/result.h>

using namespace chino::os;

HINSTANCE chino::os::hal::hal_instance;

#ifdef WIN32
#include <Windows.h>
#include <isa_availability.h>

extern "C" {
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    hal::hal_instance = hinstDLL;
    return TRUE;
}
}

#endif
