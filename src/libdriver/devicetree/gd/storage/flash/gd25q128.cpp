//
// Kernel Device
//
#include "gd25q128.hpp"
#include <kernel/device/io/Spi.hpp>
#include <kernel/device/storage/Storage.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <libbsp/bsp.hpp>
#include <string>
#include <array>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(GD25Q128Driver, "flash", "gd,gd25q128");

#define PAGE_SIZE 4 * 1024
#define PAGE_NUM 4096
#define TOTAL_SIZE PAGE_SIZE * PAGE_NUM

class GD25Q128Device : public FlashStorage, public ExclusiveObjectAccess
{
public:
	GD25Q128Device(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual size_t GetSize() override
	{
		return TOTAL_SIZE;
	}

	virtual size_t Read(size_t offset, BufferList<uint8_t> bufferList) override
	{
		if (offset >= GetSize())
			throw std::out_of_range("offset is out of range");

		return 0;
	}

	virtual void Write(size_t offset, BufferList<const uint8_t> bufferList) override
	{
		if (offset + bufferList.GetTotalSize() >= GetSize())
			throw std::out_of_range("offset is out of range");

	}
protected:
	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto spi = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("spi")->GetString(), access).MoveAs<SpiController>();
		dev_ = spi->OpenDevice(0, SpiMode::Mode0, 8, access);
	}

	virtual void OnLastClose() override
	{
		dev_.Reset();
	}
private:
private:
	const FDTDevice& fdt_;
	ObjectAccessor<SpiDevice> dev_;
};

GD25Q128Driver::GD25Q128Driver(const FDTDevice& device)
	:device_(device)
{

}

void GD25Q128Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<GD25Q128Device>(device_));
}
