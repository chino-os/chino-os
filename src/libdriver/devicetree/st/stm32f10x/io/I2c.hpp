//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class I2cDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(I2cDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
			uintptr_t regAddr_;
		};
	}
}
