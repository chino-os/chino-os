//
// Kernel Entry
//
#include <portable.h>
#include "utils.hpp"
#include "kernel_iface.h"
#include "uefigfx/BootVideo.hpp"
#include "thread/ProcessManager.hpp"
#include "memory/MemoryManager.hpp"

using namespace Chino;

StaticHolder<UefiGfx::BootVideo> g_BootVideo;
StaticHolder<Thread::ProcessManager> g_ProcessMgr;
StaticHolder<Memory::MemoryManager> g_MemoryMgr;

void Task0(uintptr_t)
{
	while (1)
	{
		PortHaltProcessor();
		for (size_t i = 0; i < 200000; i++);
		g_BootVideo->PutChar(L'0');
	}
}

void Task1(uintptr_t)
{
	while (1)
	{
		PortHaltProcessor();
		for (size_t i = 0; i < 200000; i++);
		g_BootVideo->PutChar(L'1');
	}
}

extern "C" void kernel_entry(const BootParameters* params)
{
	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);

	g_BootVideo.construct(reinterpret_cast<uint32_t*>(params->FrameBufferBase), params->FrameBufferSize, params->FrameBufferWidth, params->FrameBufferHeight);
	g_BootVideo->SetBackground(0xFF151716);
	g_BootVideo->SetMargin(20);
	g_BootVideo->ClearScreen();

	Memory::InitializeHeap(*params);

	atexit(__libc_fini_array);
	__libc_init_array();

	g_MemoryMgr.construct();

	g_BootVideo->PutString(L"Loading Chino ♥ ...\n");
	g_BootVideo->PutString(L"Natsu chan kawai ♥\n");
	g_BootVideo->PutFormat(L"Free memory avaliable: %l bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	g_ProcessMgr.construct();

	g_ProcessMgr->CreateProcess("Task 0", 1, Task0);
	g_ProcessMgr->CreateProcess("Task 1", 1, Task1);

	g_ProcessMgr->StartScheduler();
	PortHaltProcessor();
}
