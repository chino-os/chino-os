//
// Kernel Device
//
#include "at24c02.hpp"
#include <kernel/device/storage/Storage.hpp>
#include <kernel/device/io/I2c.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <libbsp/bsp.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(AT24C02Driver, "eeprom", "atmel,at24c02");

#define AT24C02PageSize 8
#define AT24C02Pages 32

class AT24C02Device final : public EEPROMStorage, public ExclusiveObjectAccess
{
public:
	AT24C02Device(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual size_t GetSize() override
	{
		return AT24C02PageSize * AT24C02Pages;
	}

	virtual size_t Read(size_t offset, BufferList<uint8_t> bufferList) override
	{
		if (offset >= GetSize())
			throw std::out_of_range("offset is out of range");

		uint8_t send[] = { static_cast<uint8_t>(offset) };
		gsl::span<const uint8_t> sendBuffers[] = { send };

		return i2cDev_->WriteRead({ sendBuffers }, bufferList);
	}

	virtual void Write(size_t offset, BufferList<const uint8_t> bufferList) override
	{
		if (offset >= GetSize())
			throw std::out_of_range("offset is out of range");

		uint8_t send[] = { static_cast<uint8_t>(offset), 0 };
		gsl::span<const uint8_t> sendBuffers[] = { send };

		for (auto& buffer : bufferList.Buffers)
		{
			for (auto data : buffer)
			{
				send[1] = data;
				i2cDev_->Write({ sendBuffers });
				BSPSleepMs(1);
				send[0]++;
			}
		}
	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;

		auto i2cName = fdt_.GetProperty("i2c");
		auto slaveAddr = fdt_.GetProperty("slave_address");
		kassert(i2cName && slaveAddr);
		auto i2c = g_ObjectMgr->GetDirectory(WKD_Device).Open(i2cName->GetString(), access).MoveAs<I2cController>();
		i2cDev_ = i2c->OpenDevice(slaveAddr->GetUInt32(0), access);
	}

	virtual void OnLastClose() override
	{
		i2cDev_.Reset();
	}
private:
	ObjectAccessor<I2cDevice> i2cDev_;
	const FDTDevice& fdt_;
};

AT24C02Driver::AT24C02Driver(const FDTDevice& device)
	:device_(device)
{

}

void AT24C02Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<AT24C02Device>(device_));
}
