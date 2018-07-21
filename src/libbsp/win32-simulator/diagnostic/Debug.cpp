//
// Kernel Diagnostic
//
#include <kernel/utils.hpp>
#include <libbsp/bsp.hpp>
#include <Windows.h>

using namespace Chino;

static HANDLE _hStdOut;

void Chino::Diagnostic::BSPInitializeDebug(const BootParameters& bootParams)
{
	_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

void Chino::Diagnostic::BSPDebugPutChar(wchar_t chr)
{
	WriteConsole(_hStdOut, &chr, 1, nullptr, nullptr);
}

void Chino::Diagnostic::BSPDebugBlueScreen()
{
}

void Chino::Diagnostic::BSPDebugClearScreen()
{
}
