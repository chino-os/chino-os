//
// Kernel Device
//
#include "Usart.hpp"
#include <kernel/kdebug.hpp>

using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(UsartDriver, "serial", "st,stm32f103-usart");

UsartDriver::UsartDriver(const FDTDevice& device)
{

}

void UsartDriver::Install()
{
	g_Logger->PutString("Usart\n");
}
