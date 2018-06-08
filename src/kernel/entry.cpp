//
// Kernel Entry
//
#include "kernel_iface.h"
#include "utils.hpp"
#include <libarch/arch.h>
#include "thread/ProcessManager.hpp"
#include "memory/MemoryManager.hpp"
#include "device/DeviceManager.hpp"
#include "diagnostic/KernelLogger.hpp"
#include "object/ObjectManager.hpp"
#include <libbsp/bsp.hpp>
#include "system/Startup.hpp"

using namespace Chino;

StaticHolder<Diagnostic::KernelLogger> g_Logger;
StaticHolder<Memory::MemoryManager> g_MemoryMgr;
StaticHolder<Thread::ProcessManager> g_ProcessMgr;
StaticHolder<Device::DeviceMananger> g_DeviceMgr;
StaticHolder<ObjectManager> g_ObjectMgr;

extern "C"
{
	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);
	extern void _init_signal(void);
}

extern "C" void Kernel_Main(const BootParameters* pParams)
{
	auto& params = *pParams;
	g_Logger.construct(params);

	Memory::MemoryManager::InitializeHeap(params);

	g_Logger->PutString(L"Loading Chino ♥ ...\n");

	atexit(__libc_fini_array);
	__libc_init_array();
	_init_signal();

	g_MemoryMgr.construct();

	g_ObjectMgr.construct();
	g_ProcessMgr.construct();
	g_DeviceMgr.construct();

	g_ProcessMgr->CreateProcess("System", 0, [&] {SystemStartup(params); });
	g_ProcessMgr->StartScheduler();

	while (1)
		ArchHaltProcessor();
}
