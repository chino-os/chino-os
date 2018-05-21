//
// Kernel Device
//
#pragma once

#include "../fdt/Fdt.hpp"

namespace Chino
{
	namespace Device
	{
		class UsartDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(UsartDriver);

			virtual void Install() override;
		};
	}
}