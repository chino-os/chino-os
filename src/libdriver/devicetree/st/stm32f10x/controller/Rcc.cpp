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

static constexpr size_t HSIFreq = 8000000;	// 8 MHz

struct rcc_cr
{
	uint32_t HSION : 1;		//!< [rw]	Internal high-speed clock enable
	uint32_t HSIRDY : 1;	//!< [r ]	Internal high-speed clock ready flag
	uint32_t RESV0 : 1;		//!<		Reserved
	uint32_t HSITRIM : 5;	//!< [rw]	Internal high-speed clock trimming
	uint32_t HSICAL : 8;	//!< [r ]	Internal high-speed clock calibration
	uint32_t HSEON : 1;		//!< [rw]	External high-speed clock enable
	uint32_t HSERDY : 1;	//!< [r ]	External high-speed clock ready flag
	uint32_t HSEBYP : 1;	//!< [rw]	External high-speed clock bypass
	uint32_t CSSON : 1;		//!< [rw]	Clock security system enable
	uint32_t RESV1 : 4;		//!<		Reserved
	uint32_t PLLON : 1;		//!< [rw]	PLL enable
	uint32_t PLLRDY : 1;	//!< [r ]	PLL clock ready flag
	uint32_t RESV2 : 6;		//!<		Reserved
};

enum sys_clk_sel
{
	SYS_CLK_SEL_HSI = 0,	//!< HSI selected as system clock
	SYS_CLK_SEL_HSE = 1,	//!< HSE selected as system clock
	SYS_CLK_SEL_PLL = 2,	//!< PLL selected as system clock
};

enum ahb_prescale
{
	AHB_CLK_SCALE_1 = 0b0000,	//!< SYSCLK not divided
	AHB_CLK_SCALE_2 = 0b1000,	//!< SYSCLK divided by 2
	AHB_CLK_SCALE_4 = 0b1001,	//!< SYSCLK divided by 4
	AHB_CLK_SCALE_8 = 0b1010,	//!< SYSCLK divided by 8
	AHB_CLK_SCALE_16 = 0b1011,	//!< SYSCLK divided by 16
	AHB_CLK_SCALE_64 = 0b1100,	//!< SYSCLK divided by 64
	AHB_CLK_SCALE_128 = 0b1101,	//!< SYSCLK divided by 128
	AHB_CLK_SCALE_256 = 0b1110,	//!< SYSCLK divided by 256
	AHB_CLK_SCALE_512 = 0b1111	//!< SYSCLK divided by 512
};

enum apb_prescale
{
	APB_CLK_SCALE_1 = 0b000,	//!< HCLK not divided
	APB_CLK_SCALE_2 = 0b100,	//!< HCLK divided by 2
	APB_CLK_SCALE_4 = 0b101,	//!< HCLK divided by 4
	APB_CLK_SCALE_8 = 0b110,	//!< HCLK divided by 8
	APB_CLK_SCALE_16 = 0b111	//!< HCLK divided by 16
};

enum adc_prescale
{
	ADC_CLK_SCALE_2 = 0b00,		//!< PCLK2 divided by 2
	ADC_CLK_SCALE_4 = 0b01,		//!< PCLK2 divided by 4
	ADC_CLK_SCALE_6 = 0b10,		//!< PCLK2 divided by 6
	ADC_CLK_SCALE_8 = 0b11		//!< PCLK2 divided by 8
};

enum pll_src_sel
{
	PLL_SRC_SEL_HSIDiv2 = 0,	//!< HSI oscillator clock / 2 selected as PLL input clock
	PLL_SRC_SEL_HSE = 1			//!< HSE oscillator clock selected as PLL input clock
};

enum hse_pll_div
{
	HSE_PLL_DIV_1 = 0,			//!< HSE clock not divided,
	HSE_PLL_DIV_2 = 1			//!< HSE clock divided by 2
};

enum pll_mul
{
	PLL_MUL_2 = 0b0000,			//!< PLL input clock x 2
	PLL_MUL_3 = 0b0001,			//!< PLL input clock x 3
	PLL_MUL_4 = 0b0010,			//!< PLL input clock x 4
	PLL_MUL_5 = 0b0011,			//!< PLL input clock x 5
	PLL_MUL_6 = 0b0100,			//!< PLL input clock x 6
	PLL_MUL_7 = 0b0101,			//!< PLL input clock x 7
	PLL_MUL_8 = 0b0110,			//!< PLL input clock x 8
	PLL_MUL_9 = 0b0111,			//!< PLL input clock x 9
	PLL_MUL_10 = 0b1000,		//!< PLL input clock x 10
	PLL_MUL_11 = 0b1001,		//!< PLL input clock x 11
	PLL_MUL_12 = 0b1010,		//!< PLL input clock x 12
	PLL_MUL_13 = 0b1011,		//!< PLL input clock x 13
	PLL_MUL_14 = 0b1100,		//!< PLL input clock x 14
	PLL_MUL_15 = 0b1101,		//!< PLL input clock x 15
	PLL_MUL_16 = 0b1110			//!< PLL input clock x 16
};

enum usb_prescale
{
	USB_CLK_SCALE_1P5 = 0,		//!< PLL clock is divided by 1.5
	USB_CLK_SCALE_1 = 1			//!< PLL clock is not divided
};

enum mco_sel
{
	MCO_SEL_NO = 0b000,			//!< No clock
	MCO_SEL_SYSCLK = 0b100,		//!< System clock (SYSCLK) selected
	MCO_SEL_HSI = 0b101,		//!< HSI clock selected
	MCO_SEL_HSE = 0b110,		//!< HSE clock selected
	MCO_SEL_PLLDiv2 = 0b111		//!< PLL clock divided by 2 selected
};

struct rcc_cfgr
{
	sys_clk_sel SW : 2;			//!< [rw]	System clock switch
	sys_clk_sel SWS : 2;		//!< [r]	System clock switch status
	ahb_prescale HPRE : 4;		//!< [rw]	AHB prescaler
	apb_prescale PPRE1 : 3;		//!< [rw]	APB low-speed prescaler (APB1)
	apb_prescale PPRE2 : 3;		//!< [rw]	APB high-speed prescaler (APB2)
	adc_prescale ADCPRE : 2;	//!< [rw]	ADC prescaler
	pll_src_sel PLLSRC : 1;		//!< [rw]	PLL entry clock source
	hse_pll_div PLLXTPRE : 1;	//!< [rw]	HSE divider for PLL entry
	pll_mul PLLMUL : 4;			//!< [rw]	PLL multiplication factor
	usb_prescale USBPRE : 1;	//!< [rw]	USB prescaler
	uint32_t RESV0 : 1;			//!<		Reserved
	mco_sel MCO : 3;			//!< [rw]	Microcontroller clock output
	uint32_t RESV1 : 5;			//!<		Reserved
};

struct apb2_enable
{
	uint32_t AFIOEN : 1;		//!< Alternate function I/O clock enable
	uint32_t RESV0 : 1;			//!< Reserved
	uint32_t IOPAEN : 1;		//!< I/O port A clock enable
	uint32_t IOPBEN : 1;		//!< I/O port B clock enable
	uint32_t IOPCEN : 1;		//!< I/O port C clock enable
	uint32_t IOPDEN : 1;		//!< I/O port D clock enable
	uint32_t IOPEEN : 1;		//!< I/O port E clock enable
	uint32_t IOPFEN : 1;		//!< I/O port F clock enable
	uint32_t IOPGEN : 1;		//!< I/O port G clock enable
	uint32_t ADC1EN : 1;		//!< ADC 1 interface clock enable
	uint32_t ADC2EN : 1;		//!< ADC 2 interface clock enable
	uint32_t TIM1EN : 1;		//!< TIM1 Timer clock enable
	uint32_t SPI1EN : 1;		//!< SPI 1 clock enable
	uint32_t TIM8EN : 1;		//!< TIM8 Timer clock enable
	uint32_t USART1EN : 1;		//!< USART1 clock enable
	uint32_t ADC3EN : 1;		//!< ADC 3 interface clock enable
	uint32_t RESV1 : 16;		//!< Reserved
};

struct apb1_enable
{
	uint32_t TIM2EN : 1;		//!< TIM2 timer clock enable
	uint32_t TIM3EN : 1;		//!< TIM3 timer clock enable
	uint32_t TIM4EN : 1;		//!< TIM4 timer clock enable
	uint32_t TIM5EN : 1;		//!< TIM5 timer clock enable
	uint32_t TIM6EN : 1;		//!< TIM6 timer clock enable
	uint32_t TIM7EN : 1;		//!< TIM7 timer clock enable
	uint32_t TIM12EN : 1;		//!< TIM12 timer clock enable
	uint32_t TIM13EN : 1;		//!< TIM13 timer clock enable
	uint32_t TIM14EN : 1;		//!< TIM14 timer clock enable
	uint32_t RESV0 : 2;			//!< Reserved
	uint32_t WWDGEN : 1;		//!< Window watchdog clock enable
	uint32_t RESV1 : 2;			//!< Reserved
	uint32_t SPI2EN : 1;		//!< SPI2 clock enable
	uint32_t SPI3EN : 1;		//!< SPI3 clock enable
	uint32_t RESV3 : 1;			//!< Reserved
	uint32_t USART2EN : 1;		//!< USART2 clock enable
	uint32_t USART3EN : 1;		//!< USART3 clock enable
	uint32_t UART4EN : 1;		//!< USART4 clock enable
	uint32_t UART5EN : 1;		//!< USART5 clock enable
	uint32_t I2C1EN : 1;		//!< I2C1 clock enable
	uint32_t I2C2EN : 1;		//!< I2C2 clock enable
	uint32_t USBEN : 1;			//!< USB clock enable
	uint32_t RESV4 : 1;			//!< Reserved
	uint32_t CANEN : 1;			//!< CAN clock enable
	uint32_t RESV5 : 1;			//!< Reserved
	uint32_t BKPEN : 1;			//!< Backup interface clock enable
	uint32_t PWREN : 1;			//!< Power interface clock enable
	uint32_t DACEN : 1;			//!< DAC interface clock enable
	uint32_t RESV6 : 2;			//!< Reserved
};

struct ahb_enable
{
	uint32_t DMA1EN : 1;		//!< DMA1 clock enable
	uint32_t DMA2EN : 1;		//!< DMA2 clock enable
	uint32_t SRAMEN : 1;		//!< SRAM interface clock enable
	uint32_t RESV0 : 1;			//!< Reserved
	uint32_t FLITFEN : 1;		//!< FLITF clock enable
	uint32_t RESV1 : 1;			//!< Reserved
	uint32_t CRCEN : 1;			//!< CRC clock enable
	uint32_t RESV2 : 1;			//!< Reserved
	uint32_t FSMCEN : 1;		//!< FSMC clock enable
	uint32_t RESV3 : 1;			//!< Reserved
	uint32_t SDIOEN : 1;		//!< SDIO clock enable
	uint32_t RESV4 : 21;		//!< Reserved
};

typedef volatile struct
{
	rcc_cr CR;
	rcc_cfgr CFGR;
	uint32_t CIR;
	uint32_t APB2RSTR;
	uint32_t APB1RSTR;
	ahb_enable AHBENR;
	apb2_enable APB2ENR;
	apb1_enable APB1ENR;
	uint32_t BDCR;
	uint32_t CSR;
} RCC_TypeDef;

#define rcc reinterpret_cast<RCC_TypeDef*>(regAddr_)

RccDriver::RccDriver(const FDTDevice& device)
	:device_(device)
{

}

void RccDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<RccDevice>(device_));
}

RccDevice::RccDevice(const FDTDevice & fdt)
{
	auto regProp = fdt.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);

	auto hseFreqProp = fdt.GetProperty("hse_freq");
	kassert(hseFreqProp.has_value());
	hseFreq_ = hseFreqProp->GetUInt32(0);

	g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
}

void RccDevice::SetPeriphClockIsEnabled(RccPeriph periph, bool enable)
{
	uint32_t value = enable ? 1 : 0;
	switch (periph)
	{
	case RccPeriph::USART1:
		rcc->APB2ENR.USART1EN = value;
		break;
	case RccPeriph::PortA:
		rcc->APB2ENR.IOPAEN = value;
		break;
	case RccPeriph::PortB:
		rcc->APB2ENR.IOPBEN = value;
		break;
	case RccPeriph::PortC:
		rcc->APB2ENR.IOPCEN = value;
		break;
	case RccPeriph::PortD:
		rcc->APB2ENR.IOPDEN = value;
		break;
	case RccPeriph::PortE:
		rcc->APB2ENR.IOPEEN = value;
		break;
	case RccPeriph::PortF:
		rcc->APB2ENR.IOPFEN = value;
		break;
	case RccPeriph::PortG:
		rcc->APB2ENR.IOPGEN = value;
		break;
	case RccPeriph::I2C1:
		rcc->APB1ENR.I2C1EN = value;
		break;
	case RccPeriph::SPI1:
		rcc->APB2ENR.SPI1EN = value;
		break;
	case RccPeriph::SPI2:
		rcc->APB1ENR.SPI2EN = value;
		break;
	case RccPeriph::FSMC:
		rcc->AHBENR.FSMCEN = value;
		break;
	case RccPeriph::DMAC1:
		rcc->AHBENR.DMA1EN = value;
		break;
	case RccPeriph::DMAC2:
		rcc->AHBENR.DMA2EN = value;
		break;
	case RccPeriph::SDIO:
		rcc->AHBENR.SDIOEN = value;
		break;
	default:
		kassert(!"invalid rcc periph.");
		break;
	}
}

void RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph periph, bool enable)
{
	auto rcc1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("rcc1", OA_Read | OA_Write).MoveAs<RccDevice>();
	rcc1->SetPeriphClockIsEnabled(periph, enable);
}

size_t RccDevice::Rcc1GetClockFrequency(RccPeriph periph)
{
	auto rcc1 = g_ObjectMgr->GetDirectory(WKD_Device).Open("rcc1", OA_Read | OA_Write).MoveAs<RccDevice>();
	return rcc1->GetClockFrequency(periph);
}

size_t RccDevice::GetClockFrequency(ClockSource clock)
{
	size_t freq = 0;

	switch (clock)
	{
	case ClockSource::HSI:
		freq = HSIFreq;
		break;
	case ClockSource::HSE:
		freq = hseFreq_;
		break;
	case ClockSource::PLL:
	{
		auto pllSrc = rcc->CFGR.PLLSRC == PLL_SRC_SEL_HSIDiv2 ? GetClockFrequency(ClockSource::HSI) / 2 : GetClockFrequency(ClockSource::HSE) / (rcc->CFGR.PLLXTPRE + 1);
		freq = pllSrc * (rcc->CFGR.PLLMUL + 2);
		break;
	}
	case ClockSource::SYSCLK:
	{
		switch (rcc->CFGR.SWS)
		{
		case SYS_CLK_SEL_HSI:
			freq = GetClockFrequency(ClockSource::HSI);
			break;
		case SYS_CLK_SEL_HSE:
			freq = GetClockFrequency(ClockSource::HSE);
			break;
		case SYS_CLK_SEL_PLL:
			freq = GetClockFrequency(ClockSource::PLL);
			break;
		}
		break;
	}
	case ClockSource::HCLK:
	{
		uint32_t divider = 0;
		switch (rcc->CFGR.HPRE)
		{
		case AHB_CLK_SCALE_1:
			divider = 1;
			break;
		case AHB_CLK_SCALE_2:
			divider = 2;
			break;
		case AHB_CLK_SCALE_4:
			divider = 4;
			break;
		case AHB_CLK_SCALE_8:
			divider = 8;
			break;
		case AHB_CLK_SCALE_16:
			divider = 16;
			break;
		case AHB_CLK_SCALE_64:
			divider = 64;
			break;
		case AHB_CLK_SCALE_128:
			divider = 128;
			break;
		case AHB_CLK_SCALE_256:
			divider = 256;
			break;
		case AHB_CLK_SCALE_512:
			divider = 512;
			break;
		}
		freq = GetClockFrequency(ClockSource::SYSCLK) / divider;
		break;
	}
	case ClockSource::PCLK1:
	{
		uint32_t divider = 0;
		switch (rcc->CFGR.PPRE1)
		{
		case APB_CLK_SCALE_1:
			divider = 1;
			break;
		case APB_CLK_SCALE_2:
			divider = 2;
			break;
		case APB_CLK_SCALE_4:
			divider = 4;
			break;
		case APB_CLK_SCALE_8:
			divider = 8;
			break;
		case APB_CLK_SCALE_16:
			divider = 16;
			break;
		}
		freq = GetClockFrequency(ClockSource::HCLK) / divider;
		break;
	}
	case ClockSource::PCLK2:
	{
		uint32_t divider = 0;
		switch (rcc->CFGR.PPRE2)
		{
		case APB_CLK_SCALE_1:
			divider = 1;
			break;
		case APB_CLK_SCALE_2:
			divider = 2;
			break;
		case APB_CLK_SCALE_4:
			divider = 4;
			break;
		case APB_CLK_SCALE_8:
			divider = 8;
			break;
		case APB_CLK_SCALE_16:
			divider = 16;
			break;
		}
		freq = GetClockFrequency(ClockSource::HCLK) / divider;
		break;
	}
	default:
		kassert(!"invalid clock source.");
		break;
	}

	return freq;
}

size_t RccDevice::GetClockFrequency(RccPeriph periph)
{
	size_t freq = 0;

	switch (periph)
	{
	case RccPeriph::I2C1:
		freq = GetClockFrequency(ClockSource::PCLK1);
		break;
	default:
		kassert(!"invalid clock source.");
		break;
	}

	return freq;
}
