//
// Kernel Device
//
#include "Usart.hpp"
#include <kernel/kdebug.hpp>

using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(UsartDriver, "serial", "st,stm32f103-usart");

UsartDriver::UsartDriver(const FDTDevice& device)
	:device_(device)
{

}

void UsartDriver::Install()
{
	auto regProp = device_.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);
	g_Logger->PutFormat("reg: %x", regAddr_);
}
