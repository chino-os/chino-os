//
// Kernel Device
//
#pragma once

#include "../pci/Pci.hpp"

namespace Chino
{
	namespace Device
	{
		class IsaDriver : public Driver
		{
		public:
			DECLARE_PCI_DRIVER(IsaDriver);

			IsaDriver(const PCIDevice& device);

			virtual void Install() override;
		private:
			volatile PCI_TYPE00* isaCfg_;
		};
	}
}
