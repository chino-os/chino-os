//
// Kernel Entry
//
#include <portable.h>
#include "utils.hpp"
#include "kernel_iface.h"
#include "uefigfx/BootVideo.hpp"
#include "thread/ProcessManager.hpp"
#include "memory/MemoryManager.hpp"
#include "device/DeviceManager.hpp"
#include "file/FileManager.hpp"

using namespace Chino;

StaticHolder<UefiGfx::BootVideo> g_BootVideo;
StaticHolder<Thread::ProcessManager> g_ProcessMgr;
StaticHolder<Memory::MemoryManager> g_MemoryMgr;
StaticHolder<Device::DeviceMananger> g_DeviceMgr;
StaticHolder<File::FileManager> g_FileMgr;

void Task0(uintptr_t)
{
	while (1)
	{
		PortHaltProcessor();
		for (size_t i = 0; i < 200000; i++);
		g_BootVideo->PutString(L"Task0 ");
	}
}

void Task1(uintptr_t)
{
	while (1)
	{
		PortHaltProcessor();
		for (size_t i = 0; i < 200000; i++);
		g_BootVideo->PutString(L"Task1 ");
	}
}

extern "C" void kernel_entry(const BootParameters* pParams)
{
	const BootParameters params = *pParams;

	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);

	g_BootVideo.construct(reinterpret_cast<uint32_t*>(params.FrameBuffer.Base), params.FrameBuffer.Size, params.FrameBuffer.Width, params.FrameBuffer.Height);
	g_BootVideo->SetBackground(0xFF151716);
	g_BootVideo->SetMargin(20);
	g_BootVideo->ClearScreen();

	Memory::InitializeHeap(params);

	atexit(__libc_fini_array);
	__libc_init_array();

	g_MemoryMgr.construct();

	g_BootVideo->PutString(L"Loading Chino ♥ ...\n");

	g_ProcessMgr.construct();
	g_DeviceMgr.construct();
	g_FileMgr.construct();

	g_DeviceMgr->InstallDevices(params);

	g_DeviceMgr->DumpDevices();
	g_FileMgr->DumpFileSystems();
	//g_ProcessMgr->CreateProcess("Task 0", 1, Task0);
	//g_ProcessMgr->CreateProcess("Task 1", 1, Task1);

	g_BootVideo->PutString(L"\nChino is successfully loaded ♥\n");
	g_BootVideo->PutFormat(L"Free memory avaliable: %l bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	auto file = g_FileMgr->OpenFile("/dev/fs0/chino/system/kernel");
	g_BootVideo->PutFormat(L"Kernel File: %lx\n", file);

	g_ProcessMgr->StartScheduler();
	PortHaltProcessor();
}
