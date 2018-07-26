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
#include <kernel/device/io/Spi.hpp>
#include <kernel/device/storage/Storage.hpp>
#include <kernel/device/sensor/Accelerometer.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

void Chino::BSPSystemStartup()
{
	auto access = OA_Read | OA_Write;
	auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open("gpio1", access).MoveAs<GpioController>();
	auto pin0 = gpio->OpenPin(0, access);
	pin0->SetDriveMode(GpioPinDriveMode::Output);

	auto eeprom1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("eeprom1", access).MoveAs<EEPROMStorage>();
	uint8_t buffer[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	{
		gsl::span<const uint8_t> writeBuffers[] = { buffer };
		eeprom1->Write(0, { writeBuffers });
	}
	{
		gsl::span<uint8_t> readBuffers[] = { buffer };
		kassert(eeprom1->Read(0, { readBuffers }) == std::size(buffer));
		g_Logger->PutString("AT24C02 Read:\n");
		g_Logger->DumpHex(buffer, std::size(buffer));
	}
	auto flash1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("flash1", access).MoveAs<FlashStorage>();
	{
		gsl::span<const uint8_t> writeBuffers[] = { buffer };
		flash1->Write(0, { writeBuffers });
	}
	{
		gsl::span<uint8_t> readBuffers[] = { buffer };
		kassert(flash1->Read(0, { readBuffers }) == std::size(buffer));
		g_Logger->PutString("GD25Q128 Read:\n");
		g_Logger->DumpHex(buffer, std::size(buffer));
	}

	auto lcd = g_ObjectMgr->GetDirectory(WKD_Device).Open("lcd1", access);

	auto accelerometer1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("accelerometer1", access).MoveAs<Accelerometer>();

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

	proc->AddThread([&]
	{
		while (true)
		{
			for (size_t i = 0; i < 100; i++)
				ArchHaltProcessor();
			auto accReading = accelerometer1->GetCurrentReading();
			g_Logger->PutFormat("Acceleration: X: %f, Y: %f, Z: %f\n", accReading.AccelerationX, accReading.AccelerationY, accReading.AccelerationZ);
		}
	}, 1);

	while (1)
		ArchHaltProcessor();
}
