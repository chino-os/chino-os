//
// Kernel Device
//
#include "DeviceManager.hpp"

#include "acpi/Acpi.hpp"

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