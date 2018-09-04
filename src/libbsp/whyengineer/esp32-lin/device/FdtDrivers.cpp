//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>

using namespace Chino::Device;

gsl::span<const uint8_t> Chino::Device::BSPGetFdtData() noexcept
{
	return {};
}

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	nullptr
};
