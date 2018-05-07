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
			enum class DeviceType
			{
				ATA,
				ATAPI
			};

			class Drive
			{

			};

			class Channel
			{
			public:
				Channel(bool isPrimary, uint32_t bar, uint32_t barCtrl);

				void Install();
			private:
				uint32_t channelId_, bar_, barCtrl_;
				DeviceType deviceType_;
			};
		public:
			DECLARE_PCI_DRIVER(IDEControllerDriver);

			IDEControllerDriver(const PCIDevice& device);

			virtual void Install() override;
		private:
			PCI_TYPE00 * ideCfg_;
			Channel channels_[2];
		};
	}
}