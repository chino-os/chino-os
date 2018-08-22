//
// Kernel Device
//
#include "gd25q128.hpp"
#include <kernel/device/io/Spi.hpp>
#include <kernel/device/io/Gpio.hpp>
#include <kernel/device/storage/Storage.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <libbsp/bsp.hpp>
#include <string>
#include <array>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(GD25Q128Driver, "flash", "gd,gd25q128");

#define PAGE_SIZE 256
#define SECTOR_SIZE (4 * 1024)
#define SECTOR_NUM 4096
#define TOTAL_SIZE (SECTOR_SIZE * SECTOR_NUM)

class GD25Q128Device : public FlashStorage, public ExclusiveObjectAccess
{
	struct CS : public ChipSelectPin
	{
		CS(ObjectAccessor<GpioPin>& pin)
			:pin_(pin)
		{
		}

		virtual void Activate() override
		{
			pin_->Write(GpioPinValue::Low);
		}

		virtual void Deactivate() override
		{
			pin_->Write(GpioPinValue::High);
		}
	private:
		ObjectAccessor<GpioPin>& pin_;
	};
public:
	GD25Q128Device(const FDTDevice& fdt)
		:fdt_(fdt), cs_(csPin_)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual uint64_t GetSize() override
	{
		return TOTAL_SIZE;
	}

	virtual size_t GetSectorSize() override
	{
		return SECTOR_SIZE;
	}

	virtual size_t GetSectorsCount() override
	{
		return SECTOR_NUM;
	}

	virtual void EraseSector(size_t sectorId) override
	{
		EraseSectorByAddress(sectorId * SECTOR_SIZE);
	}

	virtual void EraseAllSectors() override
	{
		const uint8_t cmd[] = { 0xC7 };
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		{
			WriteEnable();
			dev_->Write({ writeBuffers });
		}
	}
protected:
	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto spi = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("spi")->GetString(), access).MoveAs<SpiController>();
		dev_ = spi->OpenDevice(cs_, SpiMode::Mode0, 8, access);
		auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("cs_gpio")->GetString(), access).MoveAs<GpioController>();
		csPin_ = gpio->OpenPin(fdt_.GetProperty("cs_pin")->GetUInt32(0), access);
		csPin_->SetDriveMode(GpioPinDriveMode::Output);
		csPin_->Write(GpioPinValue::High);
	}

	virtual void OnLastClose() override
	{
		dev_.Reset();
		csPin_.Reset();
	}
private:
	void ReadSector(size_t address)
	{
		kassert(!"Not impl.");
		//gsl::span<uint8_t> buffers[] = { tempSectorSpan_ };
		//
		//auto readBuffers = bufferList.Select().Take(GetSize() - offset);
		//const uint8_t cmd[] = { 0x03, uint8_t(address >> 16), uint8_t(address >> 8), uint8_t(address) };
		//gsl::span<const uint8_t> writeBuffers[] = { cmd };
		//{
		//	BusyWait();
		//	dev_->TransferSequential({ writeBuffers }, readBuffers.AsBufferList());
		//}
		//return readBuffers.Count();
	}

	void WriteEnable()
	{
		const uint8_t cmd[] = { 0x06 };
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		{
			BusyWait();
			dev_->Write({ writeBuffers });
		}
	}

	void WriteDisable()
	{
		const uint8_t cmd[] = { 0x04 };
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		{
			BusyWait();
			dev_->Write({ writeBuffers });
		}
	}

	uint8_t ReadStatus()
	{
		const uint8_t cmd[] = { 0x05 };
		uint8_t status[1];
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		gsl::span<uint8_t> readBuffers[] = { status };
		{
			dev_->TransferSequential({ writeBuffers }, { readBuffers });
		}

		return status[0];
	}

	void BusyWait()
	{
		while (ReadStatus() & 1);
	}

	void EraseSectorByAddress(size_t address)
	{
		const uint8_t cmd[] = { 0x20, uint8_t(address >> 16), uint8_t(address >> 8), uint8_t(address) };
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		{
			WriteEnable();
			dev_->Write({ writeBuffers });
		}
	}

	void WriteSector(size_t address, BufferList<const uint8_t> bufferList)
	{
		kassert(bufferList.GetTotalSize() == SECTOR_SIZE);
		auto restBuffers = bufferList.Select();
		for (size_t i = 0; i < SECTOR_SIZE; i += PAGE_SIZE)
		{
			const uint8_t cmd[] = { 0x02, uint8_t(address >> 16), uint8_t(address >> 8), uint8_t(address) };
			auto writeBuffers = restBuffers.Take(PAGE_SIZE).Prepend(cmd);
			{
				WriteEnable();
				dev_->Write(writeBuffers.AsBufferList());
			}

			restBuffers = restBuffers.Skip(PAGE_SIZE);
			address += PAGE_SIZE;
		}
	}

	uint16_t ReadId()
	{
		const uint8_t cmd[] = { 0x90, 0, 0, 0 };
		uint16_t id;
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		gsl::span<uint8_t> readBuffers[] = { {reinterpret_cast<uint8_t*>(&id), 2 } };
		{
			dev_->TransferSequential({ writeBuffers }, { readBuffers });
		}

		return id;
	}
private:
	const FDTDevice& fdt_;
	ObjectAccessor<SpiDevice> dev_;
	ObjectAccessor<GpioPin> csPin_;
	CS cs_;
};

GD25Q128Driver::GD25Q128Driver(const FDTDevice& device)
	:device_(device)
{

}

void GD25Q128Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<GD25Q128Device>(device_));
}
