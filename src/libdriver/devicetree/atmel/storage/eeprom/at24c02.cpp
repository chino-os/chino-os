//
// Kernel Device
//
#include "at24c02.hpp"
#include <kernel/device/storage/Storage.hpp>
#include <kernel/device/io/I2c.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/object/ObjectManager.hpp>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(AT24C02Driver, "port", "atmel,at24c02");

#define AT24C02PageSize 8
#define AT24C02Pages 256

class AT24C02Device : public EEPROMStorage, public ExclusiveObjectAccess
{
public:
	AT24C02Device(const FDTDevice& device)
		:fdt_(device)
	{

	}

	virtual size_t GetSize() override
	{
		return AT24C02PageSize * AT24C02Pages;
	}

	virtual size_t Read(size_t offset, BufferList<uint8_t> bufferList) override
	{
		return 0;
	}

	virtual void Write(size_t offset, BufferList<const uint8_t> bufferList) override
	{

	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;

		auto i2cName = fdt_.GetProperty("i2c");
		auto slaveAddr = fdt_.GetProperty("slave_address");
		kassert(i2cName && slaveAddr);
		auto i2c = g_ObjectMgr->GetDirectory(WKD_Device).Open(i2cName->GetString(), access).MoveAs<I2cController>();
		i2cDev_ = i2c->OpenDevice(slaveAddr->GetUInt32(0));
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
