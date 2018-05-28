//
// Kernel Device
//
#include "Drive.hpp"
#include "../../kdebug.hpp"

using namespace Chino;
using namespace Chino::Device;

DeviceType DriveDevice::GetType() const noexcept
{
	return DeviceType::Drive;
}

ObjectPtr<Driver> DriveDevice::TryLoadDriver()
{
	auto head = g_DriveDrivers;
	auto cnt = *head;

	while (cnt)
	{
		if (cnt->IsSupported(*this))
			return cnt->Activator(*this);

		cnt = *++head;
	}

	return nullptr;
}
