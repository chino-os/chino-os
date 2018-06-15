//
// Kernel System
//
#include "Startup.hpp"
#include "../device/DeviceManager.hpp"
#include "../thread/ProcessManager.hpp"
#include "../diagnostic/KernelLogger.hpp"
#include "../memory/MemoryManager.hpp"
#include <libbsp/bsp.hpp>
#include <apis/libjs/jsi.h>

using namespace Chino;

void Chino::SystemStartup(const BootParameters& params)
{
	g_DeviceMgr->InstallDevices(params);

	g_DeviceMgr->DumpDevices();

	g_Logger->PutString(L"\nChino is successfully loaded ♥\n");
	g_Logger->PutFormat(L"Free memory avaliable: %z bytes\n", g_MemoryMgr->GetFreeBytesRemaining());

	//auto file = g_FileMgr->OpenFile("/dev/fs0/chino/system/kernel");
	//g_Logger->PutFormat(L"Opened /dev/fs0/chino/system/kernel, Size: %l bytes\n", g_FileMgr->GetFileSize(file));
	//

	auto J = js_newstate(nullptr, nullptr, 0);
	js_dostring(J, R"(
console.log('haha');
)");

	while (1)
		ArchHaltProcessor();
}
