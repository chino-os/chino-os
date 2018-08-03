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

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

void Chino::BSPSystemStartup()
{
	{
		g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());
		std::array<ObjectPtr<Mutex>, 5> mutexs;
		for (auto& m : mutexs)
			m = MakeObject<Mutex>();
		g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());
		for (auto& m : mutexs)
			m.Reset();
		g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());
	}

	auto access = OA_Read | OA_Write;
	auto lcd = g_ObjectMgr->GetDirectory(WKD_Device).Open("lcd1", access);

	while (1)
		ArchHaltProcessor();
}
