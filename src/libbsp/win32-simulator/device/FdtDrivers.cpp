//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>

using namespace Chino::Device;

const uint8_t _devicetree[1] = { 0 };

gsl::span<const uint8_t> Chino::Device::BSPGetFdtData() noexcept
{
	return _devicetree;
}

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	nullptr
};
