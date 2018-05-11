//
// Kernel Device
//
#include "Pci.hpp"
#include "../storage/IDEController.hpp"
#include "../storage/Ahci.hpp"
#include "../bridge/Isa.hpp"

using namespace Chino::Device;

#define REF_PCI_DRIVER_DESC(Type) &Type::Descriptor

const PCIDriverDescriptor* Chino::Device::g_PCIDrivers[] =
{
	//REF_PCI_DRIVER_DESC(IDEControllerDriver),
	REF_PCI_DRIVER_DESC(AhciDriver),
	REF_PCI_DRIVER_DESC(IsaDriver),
	nullptr
};
