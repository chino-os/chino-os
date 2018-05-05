//
// Kernel Device
//
#include "Pci.hpp"
#include "../storage/IDEController.hpp"

using namespace Chino::Device;

#define REF_PCI_DRIVER_DESC(Type) &Type::Descriptor

const PCIDriverDescriptor* Chino::Device::g_PCIDrivers[] =
{
	REF_PCI_DRIVER_DESC(IDEControllerDriver),
	nullptr
};
