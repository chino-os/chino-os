//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>
#include "Rcc.hpp"

namespace Chino
{
	namespace Device
	{
		class DmaChannel : public Object
		{
		public:
		};

		class DmaController : public Device
		{
		public:
			virtual ObjectPtr<DmaChannel> OpenChannel(RccPeriph periph) = 0;
		};

		class DmacDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(DmacDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
