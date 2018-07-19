//
// Kernel Device
//
#include "Fsmc.hpp"
#include "../../interface/Fsmc.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "Rcc.hpp"
#include <string>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(FsmcDriver, "fsmc", "st,stm32f103-fsmc");

class Stm32FsmcController : public FsmcController, public FreeObjectAccess
{
public:
	Stm32FsmcController(const FDTDevice& fdt)
		:fdt_(fdt)
	{

	}
private:
	const FDTDevice& fdt_;
};

FsmcDriver::FsmcDriver(const FDTDevice& device)
	:device_(device)
{

}

void FsmcDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32FsmcController>(device_));
}
