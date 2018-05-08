//
// Kernel Device
//
#include "Drive.hpp"
#include "../../kdebug.hpp"

using namespace Chino::Device;

std::unique_ptr<Driver> DriveDevice::TryLoadDriver()
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
