//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class GD25Q128Driver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(GD25Q128Driver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
