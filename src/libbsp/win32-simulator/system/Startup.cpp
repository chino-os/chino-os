//
// Kernel System
//
#include <libbsp/bsp.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ProcessManager.hpp>
#include <kernel/diagnostic/KernelLogger.hpp>
#include <kernel/memory/MemoryManager.hpp>
#include <kernel/object/ObjectManager.hpp>

using namespace Chino;
using namespace Chino::Device;

void Chino::BSPSystemStartup()
{
	while (1)
		ArchHaltProcessor();
	//auto file = g_FileMgr->OpenFile("/dev/fs0/chino/system/kernel");
	//g_Logger->PutFormat(L"Opened /dev/fs0/chino/system/kernel, Size: %l bytes\n", g_FileMgr->GetFileSize(file));
}
