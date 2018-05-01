//
// Kernel Debug
//
#pragma once
#include "uefigfx/BootVideo.hpp"

#define CHINO_WIDE(x) L##x
#define CHINO_WIDE_(x) CHINO_WIDE(x)

#define kassert(expression) \
if (!(expression)) { g_BootVideo->SetBackground(0xFF007ACC); g_BootVideo->ClearScreen(); g_BootVideo->MovePositionTo(20, 20); g_BootVideo->PutFormat(L"Assert Failed (%s, %d): %s\r\n", CHINO_WIDE_(__FILE__), __LINE__, CHINO_WIDE_(#expression)); while(1); }
