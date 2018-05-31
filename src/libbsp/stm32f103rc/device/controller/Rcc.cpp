//
// Kernel Device
//
#include "Rcc.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(RccDriver, "rcc", "st,stm32f103-rcc");

typedef volatile struct
{
	uint32_t SYSCLK_Frequency;  /*!< returns SYSCLK clock frequency expressed in Hz */
	uint32_t HCLK_Frequency;    /*!< returns HCLK clock frequency expressed in Hz */
	uint32_t PCLK1_Frequency;   /*!< returns PCLK1 clock frequency expressed in Hz */
	uint32_t PCLK2_Frequency;   /*!< returns PCLK2 clock frequency expressed in Hz */
	uint32_t ADCCLK_Frequency;  /*!< returns ADCCLK clock frequency expressed in Hz */
} RCC_ClocksTypeDef;

typedef volatile struct
{
	uint32_t CR;
	uint32_t CFGR;
	uint32_t CIR;
	uint32_t APB2RSTR;
	uint32_t APB1RSTR;
	uint32_t AHBENR;
	uint32_t APB2ENR;
	uint32_t APB1ENR;
	uint32_t BDCR;
	uint32_t CSR;
} RCC_TypeDef;

#define rcc ((RCC_TypeDef*)regAddr_)

RccDriver::RccDriver(const FDTDevice& device)
	:device_(device)
{

}

void RccDriver::Install()
{
	g_DeviceMgr->InstallDevice(*MakeObject<RccDevice>(device_));
}

RccDevice::RccDevice(const FDTDevice & fdt)
{
	auto regProp = fdt.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);

	g_ObjectMgr->GetDirectory(WKD_Device).AddItem("rcc", *this);
}

void RccDevice::SetPeriphClockIsEnabled(RccPeriph periph, bool enable)
{
	volatile uint32_t* enReg;
	uint32_t mask;
	switch (periph)
	{
	case RccPeriph::USART1:
		enReg = &rcc->APB2ENR;
		mask = 0x00004000;
		break;
	default:
		kassert(!"invalid rcc periph.");
		break;
	}

	if (enable)
		*enReg |= mask;
	else
		*enReg &= ~mask;
}
