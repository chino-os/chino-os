//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class BasicDisplayDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(BasicDisplayDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
