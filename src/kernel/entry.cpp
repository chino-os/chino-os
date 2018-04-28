//
// Kernel Entry
//
#include "kernel_iface.h"
#include <portable.h>
#include "uefigfx/BootVideo.hpp"
#include "thread/ProcessManager.hpp"

using namespace Chino;

UefiGfx::BootVideo bootVideo_;
Thread::ProcessManager processMgr_;

extern "C" void kernel_entry(const BootParameters* params)
{
	bootVideo_.Initialize(reinterpret_cast<uint32_t*>(params->FrameBufferBase), params->FrameBufferSize, params->FrameBufferWidth, params->FrameBufferHeight);
	bootVideo_.SetBackground(0xFF222222);
	bootVideo_.ClearScreen();

	bootVideo_.MovePositionTo(20, 20);
	bootVideo_.PutString(L"Loading Chino ♥ ...\r\n");
	bootVideo_.MovePositionTo(20, 40);
	bootVideo_.PutString(L"Natsu chan kawai ♥\r\n");

	processMgr_.Initialize();
	PortHaltProcessor();
}