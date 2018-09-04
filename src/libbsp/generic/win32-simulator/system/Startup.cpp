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
#include <kernel/graphics/DeviceContext.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Graphics;
using namespace Chino::Threading;

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

	App app;
	app.Start();

	auto proc = g_ProcessMgr->GetCurrentThread()->GetProcess();
	auto semp = MakeObject<Semaphore>(0);
	auto mutex = MakeObject<Mutex>();
	proc->AddThread([&]
	{
		while (true)
		{
			Locker<Mutex> locker(mutex);
			g_Logger->PutChar('1');

			semp->Take(1);

			g_Logger->PutChar('0');

			semp->Take(1);
		}
	}, 1, 512);

	proc->AddThread([&]
	{
		while (true)
		{
			for (size_t i = 0; i < 100; i++)
				ArchHaltProcessor();
			semp->Give(1);
		}
	}, 1, 512);

	while (1)
		ArchHaltProcessor();
}

void App::Start()
{
	//auto green = dc_->CreateOffscreenSurface(ColorFormat::B5G6R5_UNORM, { 10,10 });
	//dc_->Clear(*green, { {}, green->GetPixelSize() }, { 0,1,0 });
	//dc_->Clear(*primarySurface_, { {}, primarySurface_->GetPixelSize() }, { 1, 0, 0 });
	//dc_->CopySubresource(*green, *primarySurface_, { {}, green->GetPixelSize() }, { 0,0 });
}
