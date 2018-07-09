//
// Kernel Device
//
#pragma once

#include "../Fdt.hpp"

namespace Chino
{
	namespace Device
	{
		class UsartDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(UsartDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
			uintptr_t regAddr_;
		};
	}
}
