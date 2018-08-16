//
// Kernel Device
//
#include "Sdio.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Sdio.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"
#include "../controller/Dmac.hpp"
#include <libbsp/bsp.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(SdioDriver, "sdio", "st,stm32f103-sdio");

enum sdio_pwrctrl
{
	SDIO_PWR_OFF = 0b00,
	SDIO_PWR_ON = 0b11
};

struct sdio_power
{
	sdio_pwrctrl PWRCTRL : 2;		//!< Power supply control bits
	uint32_t RESV0 : 30;
};

union sdio_clkcr
{
	struct
	{
		uint32_t RESV0 : 1;
		uint32_t CLKDIV : 7;			//!< Clock divide factor
		uint32_t CLKEN : 1;				//!< Clock enable bit
		uint32_t PWRSAV : 1;			//!< Power saving configuration bit
		uint32_t BYPASS : 1;			//!< Clock divider bypass enable bit
		uint32_t WIDBUS : 2;			//!< Wide bus mode enable bit
		uint32_t NEGEDGE : 1;			//!< SDIO_CK dephasing selection bit
		uint32_t HWFC_EN : 1;			//!< HW Flow Control enable
		uint32_t RESV1 : 17;
	};

	uint32_t Value;
};

typedef volatile struct
{
	sdio_power POWER;
	sdio_clkcr CLKCR;
	uint32_t ARG;
	uint32_t CMD;
	uint32_t RESPCMD;
	uint32_t RESP[4];
	uint32_t DTIMER;
	uint32_t DLEN;
	uint32_t DCTRL;
	uint32_t DCOUNT;
	uint32_t STA;
	uint32_t ICR;
	uint32_t MASK;
	uint32_t FIFOCNT;
	uint32_t FIFO;
} SDIO_TypeDef;

class Stm32SdioController : public SdioController, public ExclusiveObjectAccess
{
public:
	Stm32SdioController(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		sdio_ = reinterpret_cast<decltype(sdio_)>(regProp->GetUInt32(0));
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void Reset() override
	{
		sdio_->POWER.PWRCTRL = SDIO_PWR_OFF;
		sdio_->CLKCR.Value = 0;
		BSPSleepMs(1);
		sdio_->CLKCR.CLKDIV = 127;		// 72M / 400K = 180
		sdio_->POWER.PWRCTRL = SDIO_PWR_ON;
		sdio_->CLKCR.CLKEN = 1;
		BSPSleepMs(1);
	}
protected:
	virtual void OnFirstOpen() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::SDIO, true);
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::SDIO, false);
	}
private:
private:
	const FDTDevice& fdt_;
	SDIO_TypeDef* sdio_;
};

SdioDriver::SdioDriver(const FDTDevice& device)
	:device_(device)
{

}

void SdioDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32SdioController>(device_));
}