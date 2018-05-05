//
// Kernel Device
//
#include "Pci.hpp"
#include "../../kdebug.hpp"

using namespace Chino::Device;

PCIDevice::PCIDevice(size_t bus, size_t device, size_t function, PCI_DEVICE_INDEPENDENT_REGION* configuration)
	:config_(configuration)
{
	g_BootVideo->PutFormat(L"Bus(%d)/Dev(%d)/Func(%d): Class(0x%x, 0x%x, 0x%x)\n", (int)bus, (int)device, (int)function, (int)configuration->ClassCode[2], (int)configuration->ClassCode[1], (int)configuration->ClassCode[0]);
}

std::unique_ptr<Driver> PCIDevice::TryLoadDriver()
{
	auto head = g_PCIDrivers;
	auto cnt = *head;
	auto classCode = config_->ClassCode;

	while (cnt)
	{
		if (classCode[1] == cnt->ClassCode[0] &&
			classCode[2] == cnt->ClassCode[1])
		{
			if (cnt->IsSupported(*this))
				return cnt->Activator(*this);
		}

		cnt = *++head;
	}

	return nullptr;
}
