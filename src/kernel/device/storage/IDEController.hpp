//
// Kernel Device
//
#pragma once

#include "../pci/Pci.hpp"

namespace Chino
{
	namespace Device
	{
		class IDEControllerDriver : public Driver
		{
		public:
			DECLARE_PCI_DRIVER(IDEControllerDriver);

			IDEControllerDriver(const PCIDevice& device);

			virtual void Install() override;
		};
	}
}