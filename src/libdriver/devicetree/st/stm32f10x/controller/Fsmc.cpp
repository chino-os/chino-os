//
// Kernel Device
//
#include "Fsmc.hpp"
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "Rcc.hpp"
#include "Port.hpp"
#include <string>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(FsmcDriver, "fsmc", "st,stm32f103-fsmc");

enum memory_type
{
	MEM_TYPE_SRAM = 0b00,		//!< SRAM (default after reset for Bank 2...4)
	MEM_TYPE_PSRAM = 0b01,		//!< PSRAM (CRAM)
	MEM_TYPE_NORFLASH = 0b10	//!< NOR Flash(default after reset for Bank 1)
};

enum databus_width
{
	DBUS_WIDTH_8 = 0b00,		//!< 8 bits
	DBUS_WIDTH_16 = 0b01		//!< 16 bits (default after reset)
};

enum wait_sig_polarity
{
	WAIT_POL_ActiveLow = 0b0,	//!< NWAIT active low
	WAIT_POL_ActiveHigh = 0b1	//!< NWAIT active high
};

union norsram_bcr
{
	struct
	{
		uint32_t MBKEN : 1;				//!< Memory bank enable bit
		uint32_t MUXEN : 1;				//!< Address/data multiplexing enable bit
		memory_type MTYP : 2;			//!< Memory type
		databus_width MWID : 2;			//!< Memory databus width
		uint32_t FACCEN : 1;			//!< Flash access enable
		uint32_t RESV0 : 1;				//!< Reserved
		uint32_t BURSTEN : 1;			//!< Burst enable bit
		wait_sig_polarity WAITPOL : 1;	//!< Wait signal polarity bit
		uint32_t WRAPMOD : 1;			//!< Wrapped burst mode support
		uint32_t WAITCFG : 1;			//!< Wait timing configuration
		uint32_t WREN : 1;				//!< Write enable bit
		uint32_t WAITEN : 1;			//!< Wait enable bit
		uint32_t EXTMOD : 1;			//!< Extended mode enable
		uint32_t RESV1 : 4;				//!< Reserved
		uint32_t CBURSTRW : 1;			//!< Write burst enable
		uint32_t RESV2 : 12;			//!< Reserved
	};

	uint32_t Value;
};

union norsram_btr
{
	struct
	{
		uint32_t ADDSET : 4;			//!< Address setup phase duration
		uint32_t ADDHLD : 4;			//!< Address-hold phase duration
		uint32_t DATAST : 8;			//!< Data-phase duration
		uint32_t BUSTURN : 4;			//!< Bus turnaround phase duration
		uint32_t CLKDIV : 4;			//!< Clock divide ratio (for CLK signal)
		uint32_t DATLAT : 4;			//!< Data latency (for synchronous burst NOR Flash)
		uint32_t ACCMOD : 2;			//!< Access mode
		uint32_t RESV0 : 2;				//!< Reserved
	};

	uint32_t Value;
};

typedef volatile struct
{
	struct
	{
		norsram_bcr BCR;
		norsram_btr BTR;
	} BTCR[4];
} Bank1_BTCR_TypeDef;

typedef volatile struct
{
	struct
	{
		norsram_btr R;
		uint32_t RESV0;
	} BWTR[4];
} Bank1_BWTR_TypeDef;

#define bank1_btcr reinterpret_cast<Bank1_BTCR_TypeDef*>(0xA0000000)
#define bank1_bwtr reinterpret_cast<Bank1_BWTR_TypeDef*>(0xA0000104)

class Stm32FsmcController : public FsmcController, public FreeObjectAccess
{
public:
	Stm32FsmcController(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto portD = g_ObjectMgr->GetDirectory(WKD_Device).Open("portD", access).MoveAs<PortDevice>();
		auto portE = g_ObjectMgr->GetDirectory(WKD_Device).Open("portE", access).MoveAs<PortDevice>();
		auto portG = g_ObjectMgr->GetDirectory(WKD_Device).Open("portG", access).MoveAs<PortDevice>();

		dxPin_[0] = portD->OpenPin(PortPins::Pin14);
		dxPin_[1] = portD->OpenPin(PortPins::Pin15);
		dxPin_[2] = portD->OpenPin(PortPins::Pin0);
		dxPin_[3] = portD->OpenPin(PortPins::Pin1);
		dxPin_[4] = portE->OpenPin(PortPins::Pin7);
		dxPin_[5] = portE->OpenPin(PortPins::Pin8);
		dxPin_[6] = portE->OpenPin(PortPins::Pin9);
		dxPin_[7] = portE->OpenPin(PortPins::Pin10);
		dxPin_[8] = portE->OpenPin(PortPins::Pin11);
		dxPin_[9] = portE->OpenPin(PortPins::Pin12);
		dxPin_[10] = portE->OpenPin(PortPins::Pin13);
		dxPin_[11] = portE->OpenPin(PortPins::Pin14);
		dxPin_[12] = portE->OpenPin(PortPins::Pin15);
		dxPin_[13] = portD->OpenPin(PortPins::Pin8);
		dxPin_[14] = portD->OpenPin(PortPins::Pin9);
		dxPin_[15] = portD->OpenPin(PortPins::Pin10);

		a10Pin_ = portG->OpenPin(PortPins::Pin0);

		noePin_ = portD->OpenPin(PortPins::Pin4);
		nwePin_ = portD->OpenPin(PortPins::Pin5);
		ne4Pin_ = portG->OpenPin(PortPins::Pin12);

		for (auto& pin : dxPin_)
			pin->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		a10Pin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		noePin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		nwePin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		ne4Pin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);

		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::FSMC, true);
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::FSMC, false);

		for (auto& pin : dxPin_)
			pin.Reset();
		a10Pin_.Reset();
		noePin_.Reset();
		nwePin_.Reset();
		ne4Pin_.Reset();
	}

	virtual void ConfigureSram(FsmcBank1Sectors sector, const NorSramTiming& readingTiming, const NorSramTiming& writingTiming) override
	{
		auto sectorId = static_cast<size_t>(sector);
		norsram_bcr bcr;
		bcr.Value = 0;
		bcr.MTYP = MEM_TYPE_SRAM;
		bcr.MWID = DBUS_WIDTH_16;
		bcr.WREN = 1;
		bcr.EXTMOD = 1;
		bank1_btcr->BTCR[sectorId].BCR.Value = bcr.Value;

		norsram_btr rtim;
		rtim.Value = 0;
		rtim.ADDSET = readingTiming.AddressSetupTime;
		rtim.ADDHLD = readingTiming.AddressHoldTime;
		rtim.DATAST = readingTiming.DataSetupTime;
		bank1_btcr->BTCR[sectorId].BTR.Value = rtim.Value;

		norsram_btr wtim;
		wtim.Value = 0;
		wtim.ADDSET = writingTiming.AddressSetupTime;
		wtim.ADDHLD = writingTiming.AddressHoldTime;
		wtim.DATAST = writingTiming.DataSetupTime;
		bank1_bwtr->BWTR[sectorId].R.Value = wtim.Value;

		bank1_btcr->BTCR[sectorId].BCR.MBKEN = 1;
	}
private:
	const FDTDevice& fdt_;
	ObjectAccessor<PortPin> dxPin_[16];
	ObjectAccessor<PortPin> a10Pin_;
	ObjectAccessor<PortPin> noePin_, nwePin_, ne4Pin_;
};

FsmcDriver::FsmcDriver(const FDTDevice& device)
	:device_(device)
{

}

void FsmcDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32FsmcController>(device_));
}

FsmcSuppress::FsmcSuppress()
{
	RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::FSMC, false);
}

FsmcSuppress::~FsmcSuppress()
{
	RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::FSMC, true);
}
