//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class AT24C02Driver : Driver
		{
		public:
			DECLARE_FDT_DRIVER(AT24C02Driver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
