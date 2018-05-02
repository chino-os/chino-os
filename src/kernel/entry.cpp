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
	g_ProcessMgr->StartScheduler();
	PortHaltProcessor();
}
