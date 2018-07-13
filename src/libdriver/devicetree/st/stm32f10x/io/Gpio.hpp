//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class GpioDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(GpioDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
