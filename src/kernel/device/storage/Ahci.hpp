//
// Kernel Device
//
#pragma once

#include "../pci/Pci.hpp"

namespace Chino
{
	namespace Device
	{
		class AhciDriver : public Driver
		{
			struct HbaFIS
			{

			};

			struct HbaPort
			{
				uint64_t BaseCmdList;
				HbaFIS* BaseFIS;
				uint32_t IS;
				uint32_t IE;
				uint32_t Cmd;
				uint32_t Reserved0;
				uint32_t TFD;
				uint32_t Sig;
				uint32_t SStaus;
				uint32_t SCtrl;
				uint32_t SErr;
				uint32_t SAct;
				uint32_t CI;
				uint32_t SNotify;
				uint32_t FBS;
				uint32_t Reserved1[11];
				uint32_t Vendor[4];
			};

			static_assert(sizeof(HbaPort) == 0x80, "Bad HbaPort layout.");

			struct HbaTable
			{
				uint32_t Cap;
				uint32_t GHC;
				uint32_t IS;
				uint32_t PI;
				uint32_t Ver;
				uint32_t CccCtrl;
				uint32_t CccPorts;
				uint32_t EmLoc;
				uint32_t EmCtrl;
				uint32_t Cap2;
				uint32_t BOHC;
				// 0x2C - 0x9F, Reserved
				uint8_t Reserved[0xA0 - 0x2C];
				// 0xA0 - 0xFF, Vendor specific registers
				uint8_t Vendor[0x100 - 0xA0];

				HbaPort Ports[32];
			};

			static_assert(sizeof(HbaTable) == 0x1100, "Bad HbaTable layout.");

			class Port
			{
			public:
				Port();

				void Install(volatile HbaPort* hbaPort);
			private:
				volatile HbaPort* hbaPort_ = nullptr;
			};
		public:
			DECLARE_PCI_DRIVER(AhciDriver);

			AhciDriver(const PCIDevice& device);

			virtual void Install() override;
		private:
			volatile PCI_TYPE00* ahicCfg_;
			volatile HbaTable* hba_;
			Port ports_[32];
		};
	}
}