//
// Kernel System
//
#include "Startup.hpp"
#include "../device/DeviceManager.hpp"
#include "../thread/ProcessManager.hpp"
#include "../diagnostic/KernelLogger.hpp"
#include "../memory/MemoryManager.hpp"
#include "../object/ObjectManager.hpp"
#include <libbsp/bsp.hpp>
#include "../device/io/Gpio.hpp"

using namespace Chino;
using namespace Chino::Device;

void Chino::SystemStartup(const BootParameters& params)
{
	g_DeviceMgr->InstallDevices(params);

	g_DeviceMgr->DumpDevices();

	g_Logger->PutString(L"\nChino is successfully loaded ♥\n");
	g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	//auto file = g_FileMgr->OpenFile("/dev/fs0/chino/system/kernel");
	//g_Logger->PutFormat(L"Opened /dev/fs0/chino/system/kernel, Size: %l bytes\n", g_FileMgr->GetFileSize(file));
	//
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

	while (1)
		ArchHaltProcessor();
}
