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
	App app;
	app.Start();

	while (1)
		g_ProcessMgr->SleepCurrentThread(1s);
}

void App::Start()
{
}
