//
// Kernel Debug
//
#pragma once
#include "uefigfx/BootVideo.hpp"

#define CHINO_WIDE(x) L##x
#define CHINO_WIDE_(x) CHINO_WIDE(x)

#define kassert(expression) \
if (!(expression)) { g_BootVideo->SetBackground(0xFF007ACC); g_BootVideo->ClearScreen(); g_BootVideo->PutFormat(L"Oops!\n\nAssert Failed: %s\nAt: %s:%d", CHINO_WIDE_(#expression), CHINO_WIDE_(__FILE__), __LINE__); while(1); }
