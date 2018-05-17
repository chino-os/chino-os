//
// Kernel Device
//
#include <libbsp/bsp.hpp>

using namespace Chino::Device;

std::unique_ptr<Driver> Chino::Device::BSPInstallRootDriver(const BootParameters& bootParams)
{
	return nullptr;
}
