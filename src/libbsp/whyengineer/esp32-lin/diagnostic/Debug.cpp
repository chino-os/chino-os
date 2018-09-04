//
// Kernel Diagnostic
//
#include <kernel/utils.hpp>
#include <libbsp/bsp.hpp>

using namespace Chino;

extern "C"
{
	extern int uart_tx_one_char(uint8_t TxChar);
}

void Chino::Diagnostic::BSPInitializeDebug(const BootParameters& bootParams)
{
}

void Chino::Diagnostic::BSPDebugPutChar(wchar_t chr)
{
	uart_tx_one_char(chr);
}

void Chino::Diagnostic::BSPDebugBlueScreen()
{
}

void Chino::Diagnostic::BSPDebugClearScreen()
{
}
