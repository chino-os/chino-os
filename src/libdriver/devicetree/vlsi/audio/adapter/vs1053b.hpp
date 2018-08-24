//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class VS1053BDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(VS1053BDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
