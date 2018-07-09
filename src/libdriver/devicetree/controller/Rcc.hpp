//
// Kernel Device
//
#pragma once

#include "../Fdt.hpp"

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
			PortG
		};

		class RccDevice : public Device, public FreeObjectAccess
		{
		public:
			RccDevice(const FDTDevice& fdt);

			void SetPeriphClockIsEnabled(RccPeriph periph, bool enable);

			static void Rcc1SetPeriphClockIsEnabled(RccPeriph periph, bool enable);
		private:
			uintptr_t regAddr_;
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