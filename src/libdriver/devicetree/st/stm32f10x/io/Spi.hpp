//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class SpiDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(SpiDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
