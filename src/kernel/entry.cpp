//
// Kernel Entry
//
#include "kernel_iface.h"
#include "utils.hpp"
#include <libarch/arch.h>
#include "threading/ProcessManager.hpp"
#include "memory/MemoryManager.hpp"
#include "device/DeviceManager.hpp"
#include "diagnostic/KernelLogger.hpp"
#include "object/ObjectManager.hpp"
#include "device/storage/filesystem/FileSystemManager.hpp"
#include <libbsp/bsp.hpp>
#include "system/Startup.hpp"

using namespace Chino;

StaticHolder<Diagnostic::KernelLogger> g_Logger;
StaticHolder<Memory::MemoryManager> g_MemoryMgr;
StaticHolder<ObjectManager> g_ObjectMgr;
StaticHolder<Threading::ProcessManager> g_ProcessMgr;
StaticHolder<Device::DeviceMananger> g_DeviceMgr;
StaticHolder<Device::FileSystemManager> g_FileSystemMgr;

extern "C"
{
	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);
}

extern "C" void Kernel_Main(const BootParameters* pParams)
{
	auto& params = *pParams;
	g_Logger.construct(params);

	Memory::MemoryManager::InitializeHeap(params);

	g_Logger->PutString(L"Loading Chino ♥ ...\n");
	atexit(__libc_fini_array);
	g_Logger->PutChar('A');
	__libc_init_array();
	g_Logger->PutChar('B');

	g_MemoryMgr.construct();

	g_ObjectMgr.construct();
	g_ProcessMgr.construct();
	g_DeviceMgr.construct();

#if 1
	g_ProcessMgr->CreateProcess("System", 1, [&] {SystemStartup(params); });
	g_ProcessMgr->StartScheduler();

	while (1)
		ArchHaltProcessor();
#else
	while (1);
#endif
}
