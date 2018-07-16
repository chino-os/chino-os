//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class ADXL345Driver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(ADXL345Driver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
