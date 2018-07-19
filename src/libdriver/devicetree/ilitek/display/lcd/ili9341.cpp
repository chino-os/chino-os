//
// Kernel Device
//
#include "ili9341.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <string>
#include <libdriver/devicetree/st/interface/Fsmc.hpp>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(ILI9341Driver, "display", "ilitek,ili9341-fsmc");

class ILI9341FsmcDevice : public Device, public ExclusiveObjectAccess
{
public:
	ILI9341FsmcDevice(const FDTDevice& fdt)
		:fdt_(fdt)
	{
	}
private:
	const FDTDevice& fdt_;
};

ILI9341Driver::ILI9341Driver(const FDTDevice& device)
	:device_(device)
{

}

void ILI9341Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<ILI9341FsmcDevice>(device_));
}
