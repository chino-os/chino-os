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
	auto access = OA_Read | OA_Write;
	auto lcd = g_ObjectMgr->GetDirectory(WKD_Device).Open("lcd1", access);

	while (1)
		ArchHaltProcessor();
}
