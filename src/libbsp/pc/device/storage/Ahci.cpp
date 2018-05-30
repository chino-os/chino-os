//
// Kernel Device
//
#include "Ahci.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <cstring>

using namespace Chino::Device;

DEFINE_PCI_DRIVER_DESC(AhciDriver, 0x01, 0x06);

AhciDriver::AhciDriver(const PCIDevice& device)
	:ahicCfg_((volatile PCI_TYPE00*)device.GetConfigurationSpace())
{
}

bool AhciDriver::IsSupported(const Chino::Device::PCIDevice& device)
{
	auto progIF = device.GetConfigurationSpace()->ClassCode[0];
	return progIF == 0x1;	// AHCI 1.0
}

#define CAP_NCS_MASK 0x1f
#define CAP_NCS_SHIFT 8

#define DEFINE_AHCI_MEM_CONSTANTS \
enum {																\
	PortCmdListSize = sizeof(HbaCmdList) * AHCI_MAX_CMD_SLOTS,		\
	PortFISSize = sizeof(HbaFIS) * AHCI_MAX_CMD_SLOTS,				\
	PortPRDTSize = sizeof(HbaCmdTable) * AHCI_MAX_CMD_SLOTS,		\
	PortMemSize = PortCmdListSize + PortFISSize + PortPRDTSize,		\
	TotalSize = PortMemSize * AHCI_MAX_PORTS						\
};

void AhciDriver::Install()
{
	hba_ = reinterpret_cast<volatile HbaTable*>(ahicCfg_->Device.Bar[5] & 0xFFFFE000);

	cmdSlotsCount_ = 1 + ((hba_->Cap >> CAP_NCS_SHIFT) & CAP_NCS_MASK);

	DEFINE_AHCI_MEM_CONSTANTS;
	hbaMemory_.reset(g_MemoryMgr->HeapAlignedAlloc(TotalSize, 1024));
	std::memset(hbaMemory_.get(), 0, TotalSize);

	//g_BootVideo->PutFormat(L"ABar: %lx, PI: %x, Slots: %d, Base: %lx\n", hba_, hba_->PI, cmdSlotsCount_, hbaMemory_.get());

	auto pi = hba_->PI;
	for (size_t i = 0; i < AHCI_MAX_PORTS; i++)
	{
		if (pi & 1)
			ports_[i].Install(i, this, hba_->Ports + i);

		pi >>= 1;
	}
}

AhciDriver::Port::Port()
{
}

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM		0x96690101	// Port multiplier

#define AHCI_DEV_NULL	0
#define AHCI_DEV_SATA	1
#define AHCI_DEV_SEMB	2
#define AHCI_DEV_PM		3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE		1
#define HBA_PORT_DET_PRESENT	3

static AhciDriver::DriveType DetectDriveType(volatile AhciDriver::HbaPort* hbaPort)
{
	using DriveType = AhciDriver::DriveType;
	auto ssts = hbaPort->SStaus;

	auto ipm = (ssts >> 8) & 0x0F;
	auto det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return DriveType::None;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return DriveType::None;

	switch (hbaPort->Sig)
	{
	case SATA_SIG_ATAPI:
		return DriveType::SATAPI;
	case SATA_SIG_SEMB:
		return DriveType::SEMB;
	case SATA_SIG_PM:
		return DriveType::PM;
	default:
		return DriveType::SATA;
	}
}

void AhciDriver::Port::Install(size_t id, AhciDriver* host, volatile HbaPort * hbaPort)
{
	id_ = id;
	host_ = host;
	hbaPort_ = hbaPort;
	driveType_ = DetectDriveType(hbaPort);

	kassert(driveType_ == DriveType::SATAPI);

	AssignSystemMemory();
	UpdateCapacity();

	static wchar_t* strs[] = { L"None", L"SATA", L"SEMB", L"PM", L"SATAPI" };
	g_Logger->PutFormat(L"Port(%d): Type: %s, Max LBA: %d, Block Size: %d\n", (int)id, strs[(int)driveType_], (int)MaxLBA, (int)BlockSize);

	g_DeviceMgr->InstallDevice(*this);
}

AhciDriver::HbaCmdList * AhciDriver::Port::TryGetFreeCommandSlot(size_t& id) const noexcept
{
	auto usedSlots = hbaPort_->SAct | hbaPort_->CI;
	auto maxSlots = host_->cmdSlotsCount_;
	for (size_t i = 0; i < maxSlots; i++)
	{
		if ((usedSlots & 1) == 0)
		{
			id = i;
			return &(*hbaPort_->BaseCmdList)[i];
		}
		usedSlots >>= 1;
	}

	return nullptr;
}

#define ATA_CMD_PACKET		0xA0
#define ATA_CMD_READ_DMA_EX 0x25
#define HBA_PxIS_TFES (1 << 30) /* TFES - Task File Error Status */

#define SCSI_CMD_READ12		0xA8
#define SCSI_CMD_READ16		0x88
#define SCSI_CMD_READ_CAPACITY12		0x25

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

void AhciDriver::Port::Read(uint64_t lba, size_t count, uint8_t * buffer)
{
	kassert(lba <= MaxLBA);
	std::array<uint8_t, 16> acmd{ 0 };
	acmd[0] = SCSI_CMD_READ12;
	acmd[2] = (lba >> 24) & 0xFF;		// most sig. byte of LBA
	acmd[3] = (lba >> 16) & 0xFF;
	acmd[4] = (lba >> 8) & 0xFF;
	acmd[5] = (lba >> 0) & 0xFF;		// least sig. byte of LBA
	acmd[8] = (count >> 8) & 0xFF;		// sectors
	acmd[9] = (count >> 0) & 0xFF;
	ExecuteScsiCommand(acmd, buffer, count * BlockSize);
}

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

void AhciDriver::Port::AssignSystemMemory()
{
	StopCommandEngine();
	DEFINE_AHCI_MEM_CONSTANTS;

	auto cmdListBase = uintptr_t(host_->hbaMemory_.get()) + PortMemSize * id_;
	auto fisBase = cmdListBase + PortCmdListSize;

	hbaPort_->BaseCmdList = reinterpret_cast<decltype(hbaPort_->BaseCmdList)>(cmdListBase);
	hbaPort_->BaseFIS = reinterpret_cast<decltype(hbaPort_->BaseFIS)>(fisBase);
	for (size_t i = 0; i < AHCI_MAX_CMD_SLOTS; i++)
	{
		auto cmdTableBase = fisBase + PortFISSize + sizeof(HbaCmdTable) * i;
		auto& slot = (*hbaPort_->BaseCmdList)[i];
		slot.PRDTL = AHCI_MAX_PRDT_ENTRIES;
		slot.CTBA = reinterpret_cast<decltype(slot.CTBA)>(cmdTableBase);
	}
	StartCommandEngine();
}

void AhciDriver::Port::StartCommandEngine()
{
	// Wait until CR (bit15) is cleared
	while (hbaPort_->Cmd & HBA_PxCMD_CR);

	// Set FRE (bit4) and ST (bit0)
	hbaPort_->Cmd |= HBA_PxCMD_FRE;
	hbaPort_->Cmd |= HBA_PxCMD_ST;
}

void AhciDriver::Port::StopCommandEngine()
{
	// Clear ST (bit0)
	hbaPort_->Cmd &= ~HBA_PxCMD_ST;

	// Wait until FR (bit14), CR (bit15) are cleared
	while (1)
	{
		if (hbaPort_->Cmd & HBA_PxCMD_FR)
			continue;
		if (hbaPort_->Cmd & HBA_PxCMD_CR)
			continue;
		break;
	}

	// Clear FRE (bit4)
	hbaPort_->Cmd &= ~HBA_PxCMD_FRE;
}

void AhciDriver::Port::ExecuteScsiCommand(const std::array<uint8_t, 16>& acmd, uint8_t * buffer, size_t size)
{
	kassert((uintptr_t(buffer) & 1) == 0);
	hbaPort_->IS = -1;
	int spin = 0; // Spin lock timeout counter
	size_t slotId;
	auto slot = TryGetFreeCommandSlot(slotId);
	kassert(slot);

	enum { SizePerPRDT = 8 * 1024 };

	auto requiredPrdtl = (size - 1) / SizePerPRDT + 1;
	kassert(requiredPrdtl <= AHCI_MAX_PRDT_ENTRIES);

	slot->CFL = sizeof(tagFIS_REG_H2D) / sizeof(uint32_t);
	slot->W = 0;
	slot->A = driveType_ == DriveType::SATAPI ? 1 : 0;
	slot->PRDTL = requiredPrdtl;

	auto& table = *slot->CTBA;
	table = {};
	for (size_t i = 0; i < requiredPrdtl - 1; i++)
	{
		auto& entry = table.prdt_entry[i];
		entry.dba = uintptr_t(buffer);
		entry.dbc = SizePerPRDT - 1;
		entry.i = 1;
		buffer += SizePerPRDT;
		size -= SizePerPRDT;
	}

	auto& lastEntry = table.prdt_entry[requiredPrdtl - 1];
	lastEntry = {};
	lastEntry.dba = uintptr_t(buffer);
	lastEntry.dbc = SizePerPRDT - 1;
	lastEntry.i = 1;

	memcpy(table.acmd, acmd.data(), sizeof(table.acmd));
	auto cmfFIS = reinterpret_cast<FIS_REG_H2D*>(table.cfis);
	cmfFIS->fis_type = FIS_TYPE_REG_H2D;
	cmfFIS->c = 1;
	cmfFIS->command = ATA_CMD_PACKET;

	// The below loop waits until the port is no longer busy before issuing a new command
	while ((hbaPort_->TFD & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 100000)
		spin++;
	if (spin == 100000)
		kassert(!"Port is hung");
	hbaPort_->CI |= 1 << slotId;

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((hbaPort_->CI & (1 << slotId)) == 0)
			break;
		if (hbaPort_->IS & HBA_PxIS_TFES)	// Task file error
			kassert(!"PRead disk error");
	}

	// Check again
	if (hbaPort_->IS & HBA_PxIS_TFES)
		kassert(!"PRead disk error");
}

void AhciDriver::Port::UpdateCapacity()
{
	std::array<uint8_t, 16> acmd{ 0 };
	acmd[0] = SCSI_CMD_READ_CAPACITY12;

	uint8_t buf[8] = { 0 };
	ExecuteScsiCommand(acmd, buf, 8);

	MaxLBA = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	BlockSize = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
}
