//
// Kernel Device
//
#pragma once

#include "../fdt/Fdt.hpp"

namespace Chino
{
	namespace Device
	{
		class RccDevice : public Device
		{
		public:
			RccDevice(const FDTDevice& fdt);
		private:
			uintptr_t regAddr_;
		};

		class RccDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(RccDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
