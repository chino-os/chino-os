//
// Kernel Device
//
#include "Ahci.hpp"
#include "../../kdebug.hpp"
#include <portable.h>

using namespace Chino::Device;

DEFINE_PCI_DRIVER_DESC(AhciDriver, 0x01, 0x06);

AhciDriver::AhciDriver(const PCIDevice& device)
	:ahicCfg_((volatile PCI_TYPE00*)device.GetConfigurationSpace())
{
}

bool AhciDriver::IsSupported(const Chino::Device::PCIDevice& device)
{
	auto progIF = device.GetConfigurationSpace()->ClassCode[0];
	return progIF == 0x1;	// AHCI 1.0
}

void AhciDriver::Install()
{
	hba_ = reinterpret_cast<volatile HbaTable*>(ahicCfg_->Device.Bar[5] & 0xFFFFE000);
	g_BootVideo->PutFormat(L"ABar: %lx, PI: %x\n", hba_, hba_->PI);

	auto pi = hba_->PI;
	for (size_t i = 0; i < 32; i++)
	{
		if (pi & 1)
		{
			auto& port = ports_[i];
			port.Install(hba_->Ports + i);
		}

		pi >>= 1;
	}
}

AhciDriver::Port::Port()
{
}

void AhciDriver::Port::Install(volatile HbaPort * hbaPort)
{
	hbaPort_ = hbaPort;
	g_BootVideo->PutFormat(L"Hba Port: %lx, Cmd Base: %lx, FIS Base: %lx\n", hbaPort, hbaPort->BaseCmdList, hbaPort->BaseFIS);
}
