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

using namespace Chino;

StaticHolder<Diagnostic::KernelLogger> g_Logger;
StaticHolder<Memory::MemoryManager> g_MemoryMgr;
StaticHolder<Thread::ProcessManager> g_ProcessMgr;
StaticHolder<Device::DeviceMananger> g_DeviceMgr;
StaticHolder<ObjectManager> g_ObjectMgr;

void Task0(uintptr_t)
{
	while (1)
	{
		Thread::BSPSleepMs(1000);
		g_Logger->PutString(L"I am task 0.\n");
	}
}

void Task1(uintptr_t)
{
	while (1)
	{
		Thread::BSPSleepMs(1000);
		g_Logger->PutString(L"I am task 1.\n");
	}
}

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

	g_DeviceMgr->InstallDevices(params);

	g_DeviceMgr->DumpDevices();

	g_ProcessMgr->CreateProcess("Task 0", 1, Task0);
	g_ProcessMgr->CreateProcess("Task 1", 1, Task1);

	g_Logger->PutString(L"\nChino is successfully loaded ♥\n");
	g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	//auto file = g_FileMgr->OpenFile("/dev/fs0/chino/system/kernel");
	//g_Logger->PutFormat(L"Opened /dev/fs0/chino/system/kernel, Size: %l bytes\n", g_FileMgr->GetFileSize(file));
	//
	g_ProcessMgr->StartScheduler();
	while (1)
		ArchHaltProcessor();
}
