//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>
#include <libdriver/devicetree/intel/display/lcd/lcd8080.hpp>

namespace Chino
{
	namespace Device
	{
		class LCD8080FsmcDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(LCD8080FsmcDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
