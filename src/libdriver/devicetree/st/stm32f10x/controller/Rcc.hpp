//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		enum class RccPeriph
		{
			USART1,
			PortA,
			PortB,
			PortC,
			PortD,
			PortE,
			PortF,
			PortG,
			I2C1,
			FSMC
		};

		class RccDevice : public Device, public FreeObjectAccess
		{
			enum class ClockSource
			{
				HSI,
				HSE,
				PLL,
				SYSCLK,
				HCLK,
				PCLK1,
				PCLK2
			};
		public:
			RccDevice(const FDTDevice& fdt);

			void SetPeriphClockIsEnabled(RccPeriph periph, bool enable);
			size_t GetClockFrequency(RccPeriph periph);

			static void Rcc1SetPeriphClockIsEnabled(RccPeriph periph, bool enable);
			static size_t Rcc1GetClockFrequency(RccPeriph periph);
		private:
			size_t GetClockFrequency(ClockSource clock);
		private:
			uintptr_t regAddr_;
			size_t hseFreq_;
		};

		class RccDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(RccDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
