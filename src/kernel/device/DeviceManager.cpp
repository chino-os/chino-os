//
// Kernel Device
//
#include "DeviceManager.hpp"

#include "acpi/Acpi.hpp"
#include "storage/Drive.hpp"

using namespace Chino::Device;

DeviceMananger::DeviceMananger()
{

}

void DeviceMananger::InstallDevices(const BootParameters& bootParams)
{
	// ACPI
	InstallDriver(std::make_unique<AcpiDriver>(bootParams));
}

void DeviceMananger::InstallDriver(std::unique_ptr<Driver>&& driver)
{
	driver->Install();
	drivers_.emplace_back(std::move(driver));
}

void DeviceMananger::RegisterDrive(DriveDevice & drive)
{
	drives_.emplace_back(drive);

	auto driver = drive.TryLoadDriver();
	if (driver)
		g_DeviceMgr->InstallDriver(std::move(driver));
}
