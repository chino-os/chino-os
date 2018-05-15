//
// Kernel Device
//
#include "Isa.hpp"
#include <kernel/kdebug.hpp>

using namespace Chino::Device;

DEFINE_PCI_DRIVER_DESC(IsaDriver, 0x06, 0x01);

IsaDriver::IsaDriver(const PCIDevice& device)
	:isaCfg_((volatile PCI_TYPE00*)device.GetConfigurationSpace())
{

}

bool IsaDriver::IsSupported(const Chino::Device::PCIDevice& device)
{
	return true;
}

struct LPC_EN
{

};

void IsaDriver::Install()
{
	auto lpcEn = (volatile LPC_EN*)(uintptr_t(isaCfg_) + 0x82);
	g_Logger->PutFormat("LPC_EN: %x\n", *reinterpret_cast<volatile uint16_t*>(lpcEn));
}
