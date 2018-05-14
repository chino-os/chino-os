//
// Kernel Device
//
#pragma once

#include "../pci/Pci.hpp"
#include <kernel/device/storage/Drive.hpp>
#include <kernel/memory/MemoryManager.hpp>

namespace Chino
{
	namespace Device
	{
		class AhciDriver : public Driver
		{
		public:
			enum AhciConstants
			{
				AHCI_MAX_PORTS = 32,
				AHCI_MAX_CMD_SLOTS = 32,
				AHCI_MAX_PRDT_ENTRIES = 8
			};

			enum FISType : uint8_t
			{
				FIS_TYPE_REG_H2D = 0x27,	// Register FIS - host to device
				FIS_TYPE_REG_D2H = 0x34,	// Register FIS - device to host
				FIS_TYPE_DMA_ACT = 0x39,	// DMA activate FIS - device to host
				FIS_TYPE_DMA_SETUP = 0x41,	// DMA setup FIS - bidirectional
				FIS_TYPE_DATA = 0x46,	// Data FIS - bidirectional
				FIS_TYPE_BIST = 0x58,	// BIST activate FIS - bidirectional
				FIS_TYPE_PIO_SETUP = 0x5F,	// PIO setup FIS - device to host
				FIS_TYPE_DEV_BITS = 0xA1,	// Set device bits FIS - device to host
			};

			typedef struct tagFIS_REG_H2D
			{
				// DWORD 0
				uint8_t  fis_type;	// FIS_TYPE_REG_H2D

				uint8_t  pmport : 4;	// Port multiplier
				uint8_t  rsv0 : 3;		// Reserved
				uint8_t  c : 1;		// 1: Command, 0: Control

				uint8_t  command;	// Command register
				uint8_t  featurel;	// Feature register, 7:0

									// DWORD 1
				uint8_t  lba0;		// LBA low register, 7:0
				uint8_t  lba1;		// LBA mid register, 15:8
				uint8_t  lba2;		// LBA high register, 23:16
				uint8_t  device;		// Device register

										// DWORD 2
				uint8_t  lba3;		// LBA register, 31:24
				uint8_t  lba4;		// LBA register, 39:32
				uint8_t  lba5;		// LBA register, 47:40
				uint8_t  featureh;	// Feature register, 15:8

									// DWORD 3
				uint8_t  countl;		// Count register, 7:0
				uint8_t  counth;		// Count register, 15:8
				uint8_t  icc;		// Isochronous command completion
				uint8_t  control;	// Control register

									// DWORD 4
				uint8_t  rsv1[4];	// Reserved
			} FIS_REG_H2D;

			typedef struct tagFIS_REG_D2H
			{
				// DWORD 0
				uint8_t  fis_type;    // FIS_TYPE_REG_D2H

				uint8_t  pmport : 4;    // Port multiplier
				uint8_t  rsv0 : 2;      // Reserved
				uint8_t  i : 1;         // Interrupt bit
				uint8_t  rsv1 : 1;      // Reserved

				uint8_t  status;      // Status register
				uint8_t  error;       // Error register

									  // DWORD 1
				uint8_t  lba0;        // LBA low register, 7:0
				uint8_t  lba1;        // LBA mid register, 15:8
				uint8_t  lba2;        // LBA high register, 23:16
				uint8_t  device;      // Device register

									  // DWORD 2
				uint8_t  lba3;        // LBA register, 31:24
				uint8_t  lba4;        // LBA register, 39:32
				uint8_t  lba5;        // LBA register, 47:40
				uint8_t  rsv2;        // Reserved

									  // DWORD 3
				uint8_t  countl;      // Count register, 7:0
				uint8_t  counth;      // Count register, 15:8
				uint8_t  rsv3[2];     // Reserved

									  // DWORD 4
				uint8_t  rsv4[4];     // Reserved
			} FIS_REG_D2H;

			typedef struct tagFIS_DATA
			{
				// DWORD 0
				uint8_t  fis_type;	// FIS_TYPE_DATA

				uint8_t  pmport : 4;	// Port multiplier
				uint8_t  rsv0 : 4;		// Reserved

				uint8_t  rsv1[2];	// Reserved

									// DWORD 1 ~ N
				uint32_t data[1];	// Payload
			} FIS_DATA;

			typedef struct tagFIS_PIO_SETUP
			{
				// DWORD 0
				uint8_t  fis_type;	// FIS_TYPE_PIO_SETUP

				uint8_t  pmport : 4;	// Port multiplier
				uint8_t  rsv0 : 1;		// Reserved
				uint8_t  d : 1;		// Data transfer direction, 1 - device to host
				uint8_t  i : 1;		// Interrupt bit
				uint8_t  rsv1 : 1;

				uint8_t  status;		// Status register
				uint8_t  error;		// Error register

									// DWORD 1
				uint8_t  lba0;		// LBA low register, 7:0
				uint8_t  lba1;		// LBA mid register, 15:8
				uint8_t  lba2;		// LBA high register, 23:16
				uint8_t  device;		// Device register

										// DWORD 2
				uint8_t  lba3;		// LBA register, 31:24
				uint8_t  lba4;		// LBA register, 39:32
				uint8_t  lba5;		// LBA register, 47:40
				uint8_t  rsv2;		// Reserved

									// DWORD 3
				uint8_t  countl;		// Count register, 7:0
				uint8_t  counth;		// Count register, 15:8
				uint8_t  rsv3;		// Reserved
				uint8_t  e_status;	// New value of status register

									// DWORD 4
				uint16_t tc;		// Transfer count
				uint8_t  rsv4[2];	// Reserved
			} FIS_PIO_SETUP;

			typedef struct tagFIS_DMA_SETUP
			{
				// DWORD 0
				uint8_t  fis_type;	// FIS_TYPE_DMA_SETUP

				uint8_t  pmport : 4;	// Port multiplier
				uint8_t  rsv0 : 1;		// Reserved
				uint8_t  d : 1;		// Data transfer direction, 1 - device to host
				uint8_t  i : 1;		// Interrupt bit
				uint8_t  a : 1;            // Auto-activate. Specifies if DMA Activate FIS is needed

				uint8_t  rsved[2];       // Reserved

										 //DWORD 1&2

				uint64_t DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

										 //DWORD 3
				uint32_t rsvd;           //More reserved

										 //DWORD 4
				uint32_t DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0

										 //DWORD 5
				uint32_t TransferCount;  //Number of bytes to transfer. Bit 0 must be 0

										 //DWORD 6
				uint32_t resvd;          //Reserved

			} FIS_DMA_SETUP;

			struct HbaFIS
			{
				// 0x00
				FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
				uint8_t         pad0[4];

				// 0x20
				FIS_PIO_SETUP	psfis;		// PIO Setup FIS
				uint8_t         pad1[12];

				// 0x40
				FIS_REG_D2H	rfis;		// Register ¨C Device to Host FIS
				uint8_t         pad2[4];

				// 0x58
				uint8_t			sdbfis;		// Set Device Bit FIS

											// 0x60
				uint8_t         ufis[64];

				// 0xA0
				uint8_t   	rsv[0x100 - 0xA0];
			};

			static_assert(sizeof(HbaFIS) == 0x100, "Bad HbaFIS layout.");

			typedef struct tagHBA_PRDT_ENTRY
			{
				uint64_t dba;		// Data base address
				uint32_t rsv0;		// Reserved

									// DW3
				uint32_t dbc : 22;		// Byte count, 4M max
				uint32_t rsv1 : 9;		// Reserved
				uint32_t i : 1;		// Interrupt on completion
			} HBA_PRDT_ENTRY;

			typedef struct tagHBA_CMD_TBL
			{
				// 0x00
				uint8_t  cfis[64];	// Command FIS

									// 0x40
				uint8_t  acmd[16];	// ATAPI command, 12 or 16 bytes

									// 0x50
				uint8_t  rsv[48];	// Reserved

									// 0x80
				HBA_PRDT_ENTRY	prdt_entry[AHCI_MAX_PRDT_ENTRIES];	// Physical region descriptor table entries, 0 ~ 65535
			} HbaCmdTable;

			struct HbaCmdList
			{
				uint32_t CFL : 5;
				uint32_t A : 1;
				uint32_t W : 1;
				uint32_t P : 1;
				uint32_t R : 1;
				uint32_t B : 1;
				uint32_t C : 1;
				uint32_t Reserved0 : 1;
				uint32_t PMP : 4;
				uint32_t PRDTL : 16;
				uint32_t PRDBC;
				HbaCmdTable* CTBA;
				uint32_t REserved1[4];
			};

			static_assert(sizeof(HbaCmdList) == 0x20, "Bad HbaCmdList layout.");

			struct HbaPort
			{
				std::array<HbaCmdList, AHCI_MAX_CMD_SLOTS>* BaseCmdList;
				std::array<volatile HbaFIS, AHCI_MAX_CMD_SLOTS>* BaseFIS;
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

				HbaPort Ports[AHCI_MAX_PORTS];
			};

			static_assert(sizeof(HbaTable) == 0x1100, "Bad HbaTable layout.");

			enum class DriveType
			{
				None,
				SATA,
				SEMB,
				PM,
				SATAPI
			};

			class Port : public DriveDevice
			{
			public:
				Port();

				void Install(size_t id, AhciDriver* host, volatile HbaPort* hbaPort);
				HbaCmdList* TryGetFreeCommandSlot(size_t& id) const noexcept;
				virtual void Read(uint64_t lba, size_t count, uint8_t* buffer) override;
			private:
				void AssignSystemMemory();
				void StartCommandEngine();
				void StopCommandEngine();

				void ExecuteScsiCommand(const std::array<uint8_t, 16>& acmd, uint8_t* buffer, size_t size);
				void UpdateCapacity();
			private:
				volatile HbaPort* hbaPort_;
				size_t id_;
				AhciDriver* host_;
				DriveType driveType_ = DriveType::None;
			};

			DECLARE_PCI_DRIVER(AhciDriver);

			AhciDriver(const PCIDevice& device);

			virtual void Install() override;
		private:
			volatile PCI_TYPE00* ahicCfg_;
			volatile HbaTable* hba_;
			size_t cmdSlotsCount_;
			Port ports_[32];
			aligned_heap_ptr<void> hbaMemory_;
		};
	}
}