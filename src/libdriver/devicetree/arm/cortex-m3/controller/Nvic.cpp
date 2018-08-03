//
// Kernel Device
//
#include "Nvic.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/controller/Pic.hpp>
#include <io_helper.hpp>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(NvicDriver, "pic", "arm,cortex-m3-nvic");

#define NVIC_ISER ((volatile uint32_t*)0xE000E100)
#define NVIC_ICER ((volatile uint32_t*)0xE000E180)
#define NVIC_ISPR ((volatile uint32_t*)0xE000E200)
#define NVIC_ICPR ((volatile uint32_t*)0xE000E280)
#define MAX_IRQ 255

class NvicDevice : public PicDevice, public FreeObjectAccess
{
public:
	NvicDevice(const FDTDevice& fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual size_t GetId() const noexcept
	{
		return 0;
	}

	virtual void SetIRQEnabled(int32_t irq, bool enable)
	{
		ValidateIRQ(irq);

		if (irq > 0)
		{
			if (enable)
				SetBit(NVIC_ISER, irq);
			else
				SetBit(NVIC_ICER, irq);
		}
	}

	virtual bool GetIRQPending(int32_t irq)
	{
		ValidateIRQ(irq);

		if (irq > 0)
			return GetBit(NVIC_ISPR, irq);
		return false;
	}

	virtual void SetIRQPending(int32_t irq, bool pending)
	{
		ValidateIRQ(irq);

		if (irq > 0)
		{
			if (pending)
				SetBit(NVIC_ISPR, irq);
			else
				SetBit(NVIC_ICPR, irq);
		}
	}
private:
	void ValidateIRQ(int32_t& irq)
	{
		kassert(irq >= -14 && irq <= MAX_IRQ);
	}
};

NvicDriver::NvicDriver(const FDTDevice& device)
	:device_(device)
{

}

void NvicDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<NvicDevice>(device_));
}
