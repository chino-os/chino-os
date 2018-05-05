//
// Kernel Device
//
#pragma once
extern "C"
{
#include <efibind.h>
#include <Acpi2_0.h>
}
#include "../Driver.hpp"
#include "../pci/Pci.hpp"
#include <vector>

namespace Chino
{
	namespace Device
	{
		class AcpiDriver : public Driver
		{
			typedef EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER RSDP_t;
		public:
			AcpiDriver(const BootParameters& bootParams);

			virtual void Install() override;
		private:
			RSDP_t * rsdp_;

			std::vector<PCIDevice> pciDevices_;
		};
	}
}
