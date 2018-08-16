//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class SdioDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(SdioDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
