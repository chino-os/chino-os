//
// Kernel System
//
#include <libbsp/bsp.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ProcessManager.hpp>
#include <kernel/diagnostic/KernelLogger.hpp>
#include <kernel/memory/MemoryManager.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <kernel/device/io/Gpio.hpp>
#include <kernel/device/io/I2c.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

void Chino::BSPSystemStartup()
{
	auto access = OA_Read | OA_Write;
	auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open("gpio1", access).MoveAs<GpioController>();
	auto pin0 = gpio->OpenPin(0, access);
	pin0->SetDriveMode(GpioPinDriveMode::Output);

	auto i2c = g_ObjectMgr->GetDirectory(WKD_Device).Open("i2c1", access).MoveAs<I2cController>();
	auto at24c02 = i2c->OpenDevice(0b1010000, access);
	
	uint8_t regAddr[] = { 0x10 };
	uint8_t buffer[10] = { 1 };

	{
		gsl::span<const uint8_t> writeBuffers[] = { regAddr, { buffer, 1 } };
		at24c02->Write({ writeBuffers });
		BSPSleepMs(1);
	}
	{
		gsl::span<const uint8_t> writeBuffers[] = { regAddr };
		gsl::span<uint8_t> readBuffers[] = { { buffer, 1 } };
		at24c02->WriteRead({ writeBuffers }, { readBuffers });
		g_Logger->PutFormat("AT24C02 Read: %d\n", buffer[0]);
	}

	auto proc = g_ProcessMgr->GetCurrentThread()->GetProcess();
	auto semp = MakeObject<Semaphore>(0);
	auto mutex = MakeObject<Mutex>();
	proc->AddThread([&]
	{
		while (true)
		{
			Locker<Mutex> locker(mutex);
			pin0->Write(GpioPinValue::Low);

			semp->Take(1);

			pin0->Write(GpioPinValue::High);

			semp->Take(1);
		}
	}, 1);

	proc->AddThread([&]
	{
		while (true)
		{
			for (size_t i = 0; i < 100; i++)
				ArchHaltProcessor();
			semp->Give(1);
		}
	}, 1);

	while (1)
		ArchHaltProcessor();
}
