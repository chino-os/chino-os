//
// Kernel Device
//
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/io/Sdio.hpp>

using namespace Chino;
using namespace Chino::Device;

class SdioRootDriver : public Driver
{
public:
	SdioRootDriver(ObjectPtr<SdioController> sdio)
		:sdio_(MakeAccessor(std::move(sdio), OA_Read | OA_Write))
	{
	}

	virtual void Install() override
	{
		EnumerateDevices();
	}
private:
	void EnumerateDevices()
	{
		sdio_->Reset();
	}
private:
	ObjectAccessor<SdioController> sdio_;
};

Chino::ObjectPtr<Driver> Chino::Device::BSPInstallSdioRootDriver(ObjectPtr<SdioController> sdio)
{
	return MakeObject<SdioRootDriver>(std::move(sdio));
}
