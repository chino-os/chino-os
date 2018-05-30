//
// Kernel Device
//
#include "Device.hpp"

using namespace Chino::Device;

Chino::ObjectPtr<Driver> Device::TryLoadDriver()
{
	return nullptr;
}

DeviceType Device::GetType() const noexcept
{
	return DeviceType::Other;
}

void Device::SetIsEnabled(bool enabled)
{
}
