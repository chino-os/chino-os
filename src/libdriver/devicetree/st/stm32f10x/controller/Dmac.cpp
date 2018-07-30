//
// Kernel Device
//
#include "Dmac.hpp"
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "Rcc.hpp"
#include <string>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_2(DmacDriver, "dmac", "st,stm32f103-dmac1", "st,stm32f103-dmac2");

template<size_t N>
class Stm32DmaController : public DmaController
{
public:
	Stm32DmaController(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual ObjectPtr<DmaChannel> OpenChannel(RccPeriph periph) override
	{
		if (N == 1)
		{

		}
		else
		{
			kassert(!"Not impl.");
		}
	}
private:
	const FDTDevice& fdt_;
};

DmacDriver::DmacDriver(const FDTDevice& device)
	:device_(device)
{

}

void DmacDriver::Install()
{
	if (device_.HasCompatible("st,stm32f103-dmac1"))
		g_DeviceMgr->InstallDevice(MakeObject<Stm32DmaController<1>>(device_));
	else if (device_.HasCompatible("st,stm32f103-dmac2"))
		g_DeviceMgr->InstallDevice(MakeObject<Stm32DmaController<2>>(device_));
}
