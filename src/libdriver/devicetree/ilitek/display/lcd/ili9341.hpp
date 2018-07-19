//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class ILI9341Driver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(ILI9341Driver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
