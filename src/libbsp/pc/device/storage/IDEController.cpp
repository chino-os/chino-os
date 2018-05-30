//
// Kernel Device
//
#include "IDEController.hpp"
#include <kernel/kdebug.hpp>
#include <libarch/arch.h>
#include <libbsp/bsp.hpp>

using namespace Chino::Device;
using namespace Chino::Thread;

DEFINE_PCI_DRIVER_DESC(IDEControllerDriver, 0x01, 0x01);

IDEControllerDriver::IDEControllerDriver(const PCIDevice & device)
	:ideCfg_((PCI_TYPE00*)device.GetConfigurationSpace()),
	channels_{
		{ true, ideCfg_->Device.Bar[0] , ideCfg_->Device.Bar[1], ideCfg_->Device.Bar[4] },
		{ false, ideCfg_->Device.Bar[2] , ideCfg_->Device.Bar[3], ideCfg_->Device.Bar[4] }
}
{
	auto cfg = device.GetConfigurationSpace();
	g_Logger->PutFormat(L"VendorId: %x, DeviceId: %x\n", cfg->VendorId, cfg->DeviceId);

	auto ideCfg = (volatile PCI_TYPE00*)cfg;

	for (int i = 0; i < 6; i++)
	{
		auto bar = ideCfg->Device.Bar + i;
		g_Logger->PutFormat(L"Bar%d: %x\n", i, *bar);
	}
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

IDEControllerDriver::Channel::Channel(bool isPrimary, uint32_t bar, uint32_t barCtrl, uint32_t busMaster)
	:base_((bar == 0 || bar == 1) ? (isPrimary ? 0x1F0 : 0x170) : bar),
	baseCtrl_((barCtrl == 0 || barCtrl == 1) ? (isPrimary ? 0x3F6 : 0x376) : bar),
	baseMaserIde_(isPrimary ? busMaster : busMaster + 8),
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
	for (uint8_t i = 0; i < 2; i++)
	{
		auto& drive = drives_[i];
		bool err = false;
		auto driveType = DriveType::ATA;

		SelectDrive(i);
		for (int i = 0; i < 4; i++)
			ReadRegister(ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.
		BSPSleepMs(10);
		g_Logger->PutFormat(L"Status:%x\n", ReadRegister(ATA_REG_STATUS));
		g_Logger->PutString(L"A");
		SendCommand(ATA_CMD_IDENTIFY);

		if (ReadRegister(ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.

		while (1) {
			auto status = ReadRegister(ATA_REG_STATUS);
			if ((status & ATA_SR_ERR)) { err = true; break; } // If Err, Device is not ATA.
			if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
		}

		if (err)
		{
			auto cl = ReadRegister(ATA_REG_LBA1);
			auto ch = ReadRegister(ATA_REG_LBA2);

			if (cl == 0x14 && ch == 0xEB)
				driveType = DriveType::ATAPI;
			else if (cl == 0x69 && ch == 0x96)
				driveType = DriveType::ATAPI;
			else
				continue; // Unknown Type (may not be a device).

			g_Logger->PutString(L"B");
			SendCommand(ATA_CMD_IDENTIFY_PACKET);
		}

		std::array<uint32_t, 128> buffer;
		g_Logger->PutString(L"C");
		ReadFifo(ATA_REG_DATA, buffer.data(), buffer.size());

		auto pBuffer = uintptr_t(buffer.data());
		drive.Type = driveType;
		drive.Signature = *reinterpret_cast<uint16_t*>(pBuffer + ATA_IDENT_DEVICETYPE);
		drive.Capabilities = *reinterpret_cast<uint16_t*>(pBuffer + ATA_IDENT_CAPABILITIES);
		drive.CommandSets = *reinterpret_cast<uint32_t*>(pBuffer + ATA_IDENT_COMMANDSETS);

		if (drive.CommandSets & (1 << 26))
			// Device uses 48-Bit Addressing:
			drive.Size = *reinterpret_cast<uint32_t*>(pBuffer + ATA_IDENT_MAX_LBA_EXT);
		else
			// Device uses CHS or 28-bit Addressing:
			drive.Size = *reinterpret_cast<uint32_t*>(pBuffer + ATA_IDENT_MAX_LBA);

		for (size_t i = 0; i < 40; i += 2)
		{
			drive.Model[i] = *reinterpret_cast<char*>(pBuffer + ATA_IDENT_MODEL + i);
			drive.Model[i + 1] = *reinterpret_cast<char*>(pBuffer + ATA_IDENT_MODEL + i);
		}
		drive.Model[40] = 0;

		g_Logger->PutString(L"D");
		static wchar_t* typeStr[] = { L"None", L"ATA", L"ATAPI" };
		std::array<wchar_t, 41> model;
		std::copy(std::begin(drive.Model), std::end(drive.Model), model.begin());

		g_Logger->PutFormat(L"Detect Drive %d: %s, Size: %d bytes, Model: %s\n", i, typeStr[(size_t)drive.Type], drive.Size, model.data());
		g_Logger->PutFormat(L"%lx\n", pBuffer);
	}
}

uint8_t IDEControllerDriver::Channel::ReadRegister(uint8_t reg)
{
	uint8_t result;
	if (reg > 0x07 && reg < 0x0C)
		WriteRegister(ATA_REG_CONTROL, 0x80 | nIEN_);
	if (reg < 0x08)
		result = ArchIOReadUInt8(base_ + reg - 0x00);
	else if (reg < 0x0C)
		result = ArchIOReadUInt8(base_ + reg - 0x06);
	else if (reg < 0x0E)
		result = ArchIOReadUInt8(baseCtrl_ + reg - 0x0A);
	else if (reg < 0x16)
		result = ArchIOReadUInt8(baseMaserIde_ + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C)
		WriteRegister(ATA_REG_CONTROL, nIEN_);
	return result;
}

void IDEControllerDriver::Channel::WriteRegister(uint8_t reg, uint8_t data)
{
	if (reg > 0x07 && reg < 0x0C)
		WriteRegister(ATA_REG_CONTROL, 0x80 | nIEN_);
	if (reg < 0x08)
		ArchIOWriteUInt8(base_ + reg - 0x00, data);
	else if (reg < 0x0C)
		ArchIOWriteUInt8(base_ + reg - 0x06, data);
	else if (reg < 0x0E)
		ArchIOWriteUInt8(baseCtrl_ + reg - 0x0A, data);
	else if (reg < 0x16)
		ArchIOWriteUInt8(baseMaserIde_ + reg - 0x0E, data);
	if (reg > 0x07 && reg < 0x0C)
		WriteRegister(ATA_REG_CONTROL, nIEN_);
}

void IDEControllerDriver::Channel::SelectDrive(uint8_t driveId)
{
	WriteRegister(ATA_REG_HDDEVSEL, 0xA0 | (driveId << 4));
	Polling();
}

void IDEControllerDriver::Channel::SendCommand(uint8_t command)
{
	WriteRegister(ATA_REG_COMMAND, command);
	Polling();
}

void IDEControllerDriver::Channel::ReadFifo(uint8_t reg, uint32_t* buffer, size_t length)
{
	if (reg > 0x07 && reg < 0x0C)
		WriteRegister(ATA_REG_CONTROL, 0x80 | nIEN_);

	if (reg < 0x08)
		ArchIOReadUInt32String(base_ + reg - 0x00, buffer, length);
	else if (reg < 0x0C)
		ArchIOReadUInt32String(base_ + reg - 0x06, buffer, length);
	else if (reg < 0x0E)
		ArchIOReadUInt32String(baseCtrl_ + reg - 0x0A, buffer, length);
	else if (reg < 0x16)
		ArchIOReadUInt32String(baseMaserIde_ + reg - 0x0E, buffer, length);

	if (reg > 0x07 && reg < 0x0C)
		WriteRegister(ATA_REG_CONTROL, nIEN_);
}

void IDEControllerDriver::Channel::Polling()
{
	// (I) Delay 400 nanosecond for BSY to be set:
	// -------------------------------------------------
	for (int i = 0; i < 4; i++)
		ReadRegister(ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

	size_t i = 0;
	// (II) Wait for BSY to be cleared:
	// -------------------------------------------------
	while (ReadRegister(ATA_REG_STATUS) & ATA_SR_BSY)
	{
		if (i++ == 10000)
		{
			i = 0;
			g_Logger->PutFormat(L"Status: %x\n", (int)ReadRegister(ATA_REG_STATUS));
		}
	}
}