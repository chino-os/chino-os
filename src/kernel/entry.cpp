//
// Kernel Entry
//
#include "utils.hpp"
#include "kernel_iface.h"
#include <libarch/arch.h>
#include "thread/ProcessManager.hpp"
#include "memory/MemoryManager.hpp"
#include "device/DeviceManager.hpp"
#include "file/FileManager.hpp"
#include "diagnostic/KernelLogger.hpp"

using namespace Chino;

StaticHolder<Diagnostic::KernelLogger> g_Logger;
StaticHolder<Thread::ProcessManager> g_ProcessMgr;
StaticHolder<Memory::MemoryManager> g_MemoryMgr;
StaticHolder<Device::DeviceMananger> g_DeviceMgr;
StaticHolder<File::FileManager> g_FileMgr;

void Task0(uintptr_t)
{
	while (1)
	{
		ArchHaltProcessor();
		for (size_t i = 0; i < 200000; i++);
		g_Logger->PutString(L"Task0 ");
	}
}

void Task1(uintptr_t)
{
	while (1)
	{
		ArchHaltProcessor();
		for (size_t i = 0; i < 200000; i++);
		g_Logger->PutString(L"Task1 ");
	}
}

extern "C" void kernel_entry(const BootParameters* pParams)
{
	const BootParameters params = *pParams;

	g_Logger.construct(params);

	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);

	Memory::InitializeHeap(params);

	atexit(__libc_fini_array);
	__libc_init_array();

	g_MemoryMgr.construct();

	g_Logger->PutString(L"Loading Chino ♥ ...\n");

	g_ProcessMgr.construct();
	g_DeviceMgr.construct();
	g_FileMgr.construct();

	g_DeviceMgr->InstallDevices(params);

	g_DeviceMgr->DumpDevices();
	g_FileMgr->DumpFileSystems();
	//g_ProcessMgr->CreateProcess("Task 0", 1, Task0);
	//g_ProcessMgr->CreateProcess("Task 1", 1, Task1);

	g_Logger->PutString(L"\nChino is successfully loaded ♥\n");
	g_Logger->PutFormat(L"Free memory avaliable: %l bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	auto file = g_FileMgr->OpenFile("/dev/fs0/chino/system/kernel");
	g_Logger->PutFormat(L"Kernel File: %lx\n", file);

	g_ProcessMgr->StartScheduler();
	ArchHaltProcessor();
}
