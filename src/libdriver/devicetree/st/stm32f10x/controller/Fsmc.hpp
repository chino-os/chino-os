//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		class FsmcDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(FsmcDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
