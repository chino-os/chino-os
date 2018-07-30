//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class NvicDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(NvicDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
