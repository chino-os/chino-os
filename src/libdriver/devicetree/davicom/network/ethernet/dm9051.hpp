//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class DM9051Driver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(DM9051Driver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
