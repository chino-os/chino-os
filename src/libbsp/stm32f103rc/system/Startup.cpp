//
// Kernel System
//
#include <libbsp/bsp.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ProcessManager.hpp>
#include <kernel/diagnostic/KernelLogger.hpp>
#include <kernel/memory/MemoryManager.hpp>
#include <kernel/network/NetworkManager.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <kernel/device/io/Gpio.hpp>
#include <kernel/device/io/I2c.hpp>
#include <kernel/device/io/Spi.hpp>
#include <kernel/device/storage/Storage.hpp>
#include <kernel/device/sensor/Accelerometer.hpp>
#include <kernel/device/network/Ethernet.hpp>
#include <kernel/graphics/DeviceContext.hpp>
#include <kernel/network/Socket.hpp>
#include <kernel/device/storage/filesystem/FileSystemManager.hpp>
#include <chrono>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Graphics;
using namespace Chino::Threading;
using namespace Chino::Network;

using namespace std::chrono_literals;

#define RW_TEST 0
#define FS_TEST 0

class App
{
public:
	App()
	{
		dc_ = MakeObject<DeviceContext>(g_ObjectMgr->GetDirectory(WKD_Device).Open("lcd1", OA_Read | OA_Write).MoveAs<DisplayDevice>());
		primarySurface_ = dc_->CreatePrimarySurface();
	}

	void Start();
private:
	ObjectPtr<DeviceContext> dc_;
	ObjectPtr<Surface> primarySurface_;
};

void Chino::BSPSystemStartup()
{
	auto access = OA_Read | OA_Write;
	auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open("gpio3", access).MoveAs<GpioController>();
	auto pin0 = gpio->OpenPin(0, access);
	pin0->SetDriveMode(GpioPinDriveMode::Output);

	auto proc = g_ProcessMgr->GetCurrentThread()->GetProcess();

#if RW_TEST
	uint8_t buffer[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	auto eeprom1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("eeprom1", access).MoveAs<EEPROMStorage>();
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

	auto accelerometer1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("accelerometer1", access).MoveAs<Accelerometer>();

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

	proc->AddThread([&]
	{
		while (true)
		{
			for (size_t i = 0; i < 100; i++)
				ArchHaltProcessor();
			auto accReading = accelerometer1->GetCurrentReading();
			g_Logger->PutFormat("Acceleration: X: %f, Y: %f, Z: %f\n", accReading.AccelerationX, accReading.AccelerationY, accReading.AccelerationZ);
		}
	}, 1, 2048);
#endif

	auto semp = MakeObject<Semaphore>(0);
	auto mutex = MakeObject<Mutex>();
	proc->AddThread([&]
	{
		while (true)
		{
			Locker<Mutex> locker(mutex);

			if (semp->TryTake(1, 1s))
				pin0->Write(GpioPinValue::High);
			else
				pin0->Write(GpioPinValue::Low);
		}
	}, 1, 512);

	proc->AddThread([&]
	{
		while (true)
		{
			g_ProcessMgr->SleepCurrentThread(5s);
			semp->Give(1);
		}
	}, 1, 512);

	g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	App app;
	app.Start();

	while (1)
		g_ProcessMgr->SleepCurrentThread(1s);
}

void App::Start()
{
	auto green = dc_->CreateOffscreenSurface(ColorFormat::B5G6R5_UNORM, { 30,30 });
	dc_->Clear(*green, { {}, green->GetPixelSize() }, { 0,1,0 });
	dc_->Clear(*primarySurface_, { {}, primarySurface_->GetPixelSize() }, { 1, 0, 0 });
	dc_->CopySubresource(*green, *primarySurface_, { {}, green->GetPixelSize() }, { 100, 100 });

	auto eth = g_NetworkMgr->InstallNetworkDevice(g_ObjectMgr->GetDirectory(WKD_Device).Open("eth1", OA_Read | OA_Write).MoveAs<EthernetController>());
	eth->SetAsDefault();
	eth->Setup();
	g_NetworkMgr->Run();

#if FS_TEST
	g_FileSystemMgr->Mount("0:", g_ObjectMgr->GetDirectory(WKD_Device).Open("sd0", OA_Read | OA_Write).MoveAs<SDStorage>());
	auto file = g_FileSystemMgr->OpenFile("0:/setup.exe", FileAccess::Read);
	g_Logger->PutFormat("setup.exe Size: %z bytes\n", file->GetSize());
#endif

	auto bindAddr = std::make_shared<IPEndPoint>(IPAddress::IPv4Any, 80);
	auto socket = MakeObject<Socket>(AddressFamily::IPv4, SocketType::Stream, ProtocolType::Tcp);
	socket->Bind(bindAddr);
	socket->Listen(1);

	auto client = socket->Accept();
	while (true)
	{
		const uint8_t text[] = "hello\n";
		gsl::span<const uint8_t> buffers[] = { {text,6} };
	
		try
		{
			client->Send({ buffers });
			g_ProcessMgr->SleepCurrentThread(1s);
		}
		catch (...)
		{
			break;
		}
	}
}
