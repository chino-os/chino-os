//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class ENC28J60Driver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(ENC28J60Driver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
