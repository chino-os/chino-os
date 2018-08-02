//
// Kernel Device
//
#include "DeviceManager.hpp"
#include "../kdebug.hpp"
#include <libbsp/bsp.hpp>
#include "controller/Pic.hpp"
#include "../threading/ThreadSynchronizer.hpp"

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

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

ObjectPtr<IObject> DeviceMananger::InstallIRQHandler(size_t picId, int32_t irq, std::function<void()> handler)
{
	kernel_critical kc;
	auto entry = MakeObject<IRQEntry>(picId, irq, std::move(handler));
	irqHandlers_[picId].emplace(irq, entry.Get());
	return entry;
}

void DeviceMananger::OnIRQ(size_t picId, int32_t irq)
{
	auto& handler = irqHandlers_.at(picId).at(irq);
	handler->Invoke();
}

void DeviceMananger::UninstallIRQHandler(size_t picId, int32_t irq)
{
	kernel_critical kc;
	irqHandlers_.at(picId).erase(irq);
}

DeviceMananger::IRQEntry::IRQEntry(size_t picId, int32_t irq, std::function<void()>&& handler)
	:picId_(picId), irq_(irq), handler_(std::move(handler))
{
	kassert(handler_);
}

DeviceMananger::IRQEntry::~IRQEntry()
{
	g_DeviceMgr->UninstallIRQHandler(picId_, irq_);
}

void DeviceMananger::IRQEntry::Invoke()
{
	handler_();
}
