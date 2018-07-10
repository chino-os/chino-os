//
// Kernel System
//
#include <libbsp/bsp.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ProcessManager.hpp>
#include <kernel/diagnostic/KernelLogger.hpp>
#include <kernel/memory/MemoryManager.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/io/Gpio.hpp>

using namespace Chino;
using namespace Chino::Device;

void Chino::BSPSystemStartup()
{
	auto access = OA_Read | OA_Write;
	auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open("gpio0", access).MoveAs<GpioController>();
	auto pin0 = gpio->OpenPin(0, access);
	pin0->SetDriveMode(GpioPinDriveMode::Output);

	while (true)
	{
		pin0->Write(GpioPinValue::Low);

		for (size_t i = 0; i < 100; i++)
			ArchHaltProcessor();

		pin0->Write(GpioPinValue::High);

		for (size_t i = 0; i < 100; i++)
			ArchHaltProcessor();
	}
}
