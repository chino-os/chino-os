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
#include <libdriver/devicetree/vlsi/audio/adapter/vs1053b.hpp>

using namespace Chino;
using namespace Chino::Audio;
using namespace Chino::Device;
using namespace Chino::Graphics;
using namespace Chino::Threading;
using namespace Chino::Network;

using namespace std::chrono_literals;

#define RW_TEST 0
#define FS_TEST 1
#define AUDIO_TEST 1
#define AUDIO_TEST_RENDER 1
#define NET_TEST 0
#define LCD_TEST 0

class App
{
public:
	App()
	{
#if LCD_TEST
		dc_ = MakeObject<DeviceContext>(g_ObjectMgr->GetDirectory(WKD_Device).Open("lcd1", OA_Read | OA_Write).MoveAs<DisplayDevice>());
		primarySurface_ = dc_->CreatePrimarySurface();
#endif
	}

	void Start();
private:
#if LCD_TEST
	ObjectPtr<DeviceContext> dc_;
	ObjectPtr<Surface> primarySurface_;
#endif
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
#if LCD_TEST
	auto green = dc_->CreateOffscreenSurface(ColorFormat::B5G6R5_UNORM, { 30,30 });
	dc_->Clear(*green, { {}, green->GetPixelSize() }, { 0,1,0 });
	dc_->Clear(*primarySurface_, { {}, primarySurface_->GetPixelSize() }, { 1, 0, 0 });
	dc_->CopySubresource(*green, *primarySurface_, { {}, green->GetPixelSize() }, { 100, 100 });
#endif

#if NET_TEST
	g_NetworkMgr.construct();
	auto eth = g_NetworkMgr->InstallNetworkDevice(g_ObjectMgr->GetDirectory(WKD_Device).Open("eth0", OA_Read | OA_Write).MoveAs<EthernetController>());
	eth->SetAsDefault();
	eth->Setup();
	g_NetworkMgr->Run();
#endif

#if FS_TEST
	g_FileSystemMgr.construct();
	g_FileSystemMgr->Mount("0:", g_ObjectMgr->GetDirectory(WKD_Device).Open("sd0", OA_Read | OA_Write).MoveAs<SDStorage>());

#if AUDIO_TEST
	auto audioClient = CreateVS1053BAudioClient(g_ObjectMgr->GetDirectory(WKD_Device).Open("audio0", OA_Read | OA_Write).MoveAs<Chino::Device::Device>());
	audioClient->SetChannelVolume(0, 0.6f);
	audioClient->SetChannelVolume(1, 0.6f);
#if AUDIO_TEST_RENDER
	AudioFormat format{ AudioFormatTag::AutoDetect };
	audioClient->SetMode(AudioClientMode::Render);
	audioClient->SetFormat(format);
	auto render = audioClient->GetRenderClient();
	auto file = g_FileSystemMgr->OpenFile("0:/MUSIC/badapple.flac", FileAccess::Read);
	g_Logger->PutFormat("badapple.flac Size: %z bytes\n", file->GetSize());

	// loop
	while (true)
	{
		audioClient->Start();
		file->SetPosition(0);
		gsl::span<uint8_t> buffers[1];
		bool end = false;
		while (!end)
		{
			render->GetBuffer(buffers[0]);
			if (file->Read({ buffers }) == 0)
				end = true;
			render->ReleaseBuffer();
		}
		audioClient->Stop();
	}
#else
	struct RIFF
	{
		uint32_t ChunkID;
		uint32_t ChunkSize;
		uint32_t Format;
		uint32_t SubChunk1ID;
		uint32_t SubChunk1Size;
		uint16_t AudioFormat;
		uint16_t NumOfChannels;
		uint32_t SampleRate;
		uint32_t ByteRate;
		uint16_t BlockAlign;
		uint16_t BitsPerSample;
		uint32_t SubChunk3ID;
		uint32_t SubChunk3Size;
	} riff;

	riff.ChunkID = 'FFIR';
	riff.Format = 'EVAW';
	riff.SubChunk1ID = ' tmf';
	riff.SubChunk1Size = 0x10;
	riff.AudioFormat = 1;
	riff.NumOfChannels = 1;
	riff.SampleRate = 8000;
	riff.ByteRate = 8000 * 1 * 2;
	riff.BlockAlign = 2;
	riff.BitsPerSample = 16;
	riff.SubChunk3ID = 'atad';

	gsl::span<const uint8_t> riffbuf[] = { {reinterpret_cast<const uint8_t*>(&riff), sizeof(riff)} };

	AudioFormat format{ AudioFormatTag::PCM, 1, 8000, 2, 16 };
	audioClient->SetMode(AudioClientMode::Capture);
	audioClient->SetFormat(format);

	auto capture = audioClient->GetCaptureClient();
	const auto maxSecs = 5;
	{
		auto file = g_FileSystemMgr->OpenFile("0:/MUSIC/record.wav", FileAccess::Write, FileMode::CreateAlways);
		file->Write({ riffbuf });

		size_t written = 0;
		gsl::span<const uint8_t> buffers[1];
		audioClient->Start();
		while (written < 2 * 8000 * maxSecs)
		{
			capture->GetBuffer(buffers[0]);
			if (!buffers[0].empty())
			{
				file->Write({ buffers });
				written += buffers[0].size();
			}
			capture->ReleaseBuffer();
		}
		auto size = sizeof(riff) + written;
		file->SetPosition(0);
		riff.ChunkSize = size - 8;
		riff.SubChunk3Size = size - 36;
		file->Write({ riffbuf });
	}

	g_Logger->PutString("Capture done.\n");
#endif
#endif
#endif

#if NET_TEST
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
#endif
}
