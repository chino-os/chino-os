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
			enum class DriveType
			{
				None,
				ATA,
				ATAPI
			};

			struct Drive
			{
				DriveType Type = DriveType::None;
				uint16_t Signature;
				uint16_t Capabilities;
				uint32_t CommandSets;
				uint32_t Size;
				char Model[41];
			};

			class Channel
			{
			public:
				Channel(bool isPrimary, uint32_t bar, uint32_t barCtrl, uint32_t busMaster);

				void Install();
			private:
				uint8_t ReadRegister(uint8_t reg);
				void WriteRegister(uint8_t reg, uint8_t data);
				void ReadFifo(uint8_t reg, uint32_t* buffer, size_t length);
				void Polling();

				void SelectDrive(uint8_t driveId);
				void SendCommand(uint8_t command);
			private:
				uint32_t channelId_, base_, baseCtrl_, baseMaserIde_;
				uint8_t nIEN_;
				Drive drives_[2];
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