//
// Kernel Device
//
#include "DeviceManager.hpp"
#include "../kdebug.hpp"
#include <libbsp/bsp.hpp>

using namespace Chino;
using namespace Chino::Device;

DeviceMananger::DeviceMananger()
{
}

void DeviceMananger::InstallDevices(const BootParameters& bootParams)
{
	// Root
	auto root = BSPInstallRootDriver(bootParams);
	if (root)
		InstallDriver(root);
}

void DeviceMananger::InstallDriver(ObjectPtr<Driver> driver)
{
	driver->Install();
	drivers_.emplace_back(std::move(driver));
}

void DeviceMananger::InstallDevice(ObjectPtr<Chino::Device::Device> drive)
{
	devices_.emplace_back(drive);

	auto driver = drive->TryLoadDriver();
	if (driver)
		InstallDriver(std::move(driver));
}

void DeviceMananger::DumpDevices()
{
	//g_Logger->PutString("====== Dump Devices ======\n");
	//int i = 0;
	//for (auto& dev : drives_)
	//{
	//	g_Logger->PutFormat("Drive%d: Max LBA: %d, Block Size: %d\n", i++, (int)dev->MaxLBA, (int)dev->BlockSize);
	//}
}
