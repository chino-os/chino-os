//
// Kernel Device
//
#include "IDEController.hpp"
#include "../../kdebug.hpp"

using namespace Chino::Device;

DEFINE_PCI_DRIVER_DESC(IDEControllerDriver, 0x01, 0x01);

IDEControllerDriver::IDEControllerDriver(const PCIDevice & device)
	:ideCfg_((PCI_TYPE00*)device.GetConfigurationSpace()),
	channels_ { 
		{ true, ideCfg_->Device.Bar[0] , ideCfg_->Device.Bar[1] },
		{ false, ideCfg_->Device.Bar[2] , ideCfg_->Device.Bar[3] }
	}
{
	auto cfg = device.GetConfigurationSpace();
	g_BootVideo->PutFormat(L"VendorId: %x, DeviceId: %x\n", cfg->VendorId, cfg->DeviceId);

	auto ideCfg = (PCI_TYPE00*)cfg;
	g_BootVideo->PutFormat(L"Bar0: %x\n", ideCfg->Device.Bar[0]);
	g_BootVideo->PutFormat(L"Bar1: %x\n", ideCfg->Device.Bar[1]);
	g_BootVideo->PutFormat(L"Bar2: %x\n", ideCfg->Device.Bar[2]);
	g_BootVideo->PutFormat(L"Bar3: %x\n", ideCfg->Device.Bar[3]);
	g_BootVideo->PutFormat(L"Bar4: %x\n", ideCfg->Device.Bar[4]);
}

bool IDEControllerDriver::IsSupported(const Chino::Device::PCIDevice& device)
{
	return true;
}

void IDEControllerDriver::Install()
{
	for (auto& channel : channels_)
		channel.Install();
}

IDEControllerDriver::Channel::Channel(bool isPrimary, uint32_t bar, uint32_t barCtrl)
	:bar_((bar == 0 || bar == 1) ? (isPrimary ? 0x1F0 : 0x170) : bar),
	barCtrl_((barCtrl == 0 || barCtrl == 1) ? (isPrimary ? 0x3F6 : 0x376) : bar),
	channelId_(isPrimary ? 0 : 1)
{
}

// Status
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Inlex
#define ATA_SR_ERR     0x01    // Error

// Errors
#define ATA_ER_BBK      0x80    // Bad sector
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // No media
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // No media
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// Commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATAPI_CMD_READ			  0xA8
#define ATAPI_CMD_EJECT			  0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

void IDEControllerDriver::Channel::Install()
{

}