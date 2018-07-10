//
// Kernel Device
//
#include <libdriver/acpi/storage/Ahci.hpp>
#include <libdriver/acpi/bridge/Isa.hpp>

using namespace Chino::Device;

#define REF_PCI_DRIVER_DESC(Type) &Type::Descriptor

const PCIDriverDescriptor* Chino::Device::g_PCIDrivers[] =
{
	REF_PCI_DRIVER_DESC(AhciDriver),
	REF_PCI_DRIVER_DESC(IsaDriver),
	nullptr
};
