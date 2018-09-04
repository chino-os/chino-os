//
// Kernel Diagnostic
//
#include <kernel/utils.hpp>
#include <libbsp/bsp.hpp>
#include "../bsp_defines.hpp"
#include "BootVideo.hpp"

using namespace Chino;

StaticHolder<UefiGfx::BootVideo> g_BootVideo;

void Chino::Diagnostic::BSPInitializeDebug(const BootParameters& bootParams)
{
	g_BootVideo.construct(reinterpret_cast<uint32_t*>(bootParams.FrameBuffer.Base), bootParams.FrameBuffer.Size, bootParams.FrameBuffer.Width, bootParams.FrameBuffer.Height);

	g_BootVideo->SetBackground(0xFF151716);
	g_BootVideo->SetMargin(20);
	g_BootVideo->ClearScreen();
}

void Chino::Diagnostic::BSPDebugPutChar(wchar_t chr)
{
	g_BootVideo->PutChar(chr);
}

void Chino::Diagnostic::BSPDebugBlueScreen()
{
	g_BootVideo->SetBackground(0xFF007ACC);
	g_BootVideo->ClearScreen();
}

void Chino::Diagnostic::BSPDebugClearScreen()
{
	g_BootVideo->ClearScreen();
}
