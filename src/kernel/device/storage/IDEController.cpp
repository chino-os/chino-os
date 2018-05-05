//
// Kernel Device
//
#include "IDEController.hpp"
#include "../../kdebug.hpp"

using namespace Chino::Device;

DEFINE_PCI_DRIVER_DESC(IDEControllerDriver, 0x01, 0x01);

IDEControllerDriver::IDEControllerDriver(const PCIDevice & device)
{
}

bool IDEControllerDriver::IsSupported(const Chino::Device::PCIDevice& device)
{
	auto cfg = device.GetConfigurationSpace();
	g_BootVideo->PutFormat(L"VendorId: %x, DeviceId: %x\n", cfg->VendorId, cfg->DeviceId);

	auto ideCfg = (PCI_TYPE00*)cfg;
	g_BootVideo->PutFormat(L"Bar0: %x\n", ideCfg->Device.Bar[0]);
	g_BootVideo->PutFormat(L"Bar1: %x\n", ideCfg->Device.Bar[1]);
	g_BootVideo->PutFormat(L"Bar2: %x\n", ideCfg->Device.Bar[2]);
	g_BootVideo->PutFormat(L"Bar3: %x\n", ideCfg->Device.Bar[3]);
	g_BootVideo->PutFormat(L"Bar4: %x\n", ideCfg->Device.Bar[4]);

	return false;
}

void IDEControllerDriver::Install()
{
}
