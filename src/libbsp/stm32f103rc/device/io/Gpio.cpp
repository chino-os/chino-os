//
// Kernel Device
//
#include "Gpio.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/io/Gpio.hpp>
#include "../controller/Port.hpp"

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(GpioDriver, "gpio", "st,stm32f103-gpio");

class Stm32GpioPin : public GpioPin, public ExclusiveObjectAccess
{
public:
	Stm32GpioPin(ObjectAccessor<PortPin>&& portPin)
		:portPin_(std::move(portPin))
	{

	}

	virtual GpioPinValue Read() override
	{
		return GpioPinValue::Low;
	}

	virtual void Write(GpioPinValue value) override
	{

	}

	virtual void SetDriveMode(GpioPinDriveMode driveMode) override
	{
		bool input;
		PortInputMode imode;
		PortOutputMode omode;

		switch (driveMode)
		{
		case GpioPinDriveMode::Input:
			input = true;
			imode = PortInputMode::Floating;
			break;
		case GpioPinDriveMode::InputPullDown:
			input = true;
			imode = PortInputMode::PullDown;
			break;
		case GpioPinDriveMode::InputPullUp:
			input = true;
			imode = PortInputMode::PullUp;
			break;
		case GpioPinDriveMode::Output:
			input = false;
			omode = PortOutputMode::PushPull;
			break;
		default:
			kassert(!"Invalid drive mode.");
			break;
		}

		if (input)
			portPin_->SetMode(imode);
		else
			portPin_->SetMode(omode, PortOutputSpeed::PS_2MHz);
	}
private:
	ObjectAccessor<PortPin> portPin_;
};

class Stm32GpioController : public GpioController, public FreeObjectAccess
{
public:
	Stm32GpioController(const FDTDevice& device)
		:portDevice_(g_ObjectMgr->GetDirectory(WKD_Device)
			.Open(device.GetProperty("port")->GetString(), OA_Read | OA_Write).MoveAs<PortDevice>())
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(device.GetName(), *this);
	}

	virtual size_t GetPinCount() const override
	{
		return PortDevice::PinCount;
	}

	virtual ObjectAccessor<GpioPin> OpenPin(size_t index, ObjectAccess access) override
	{
		return MakeAccessor(MakeObject<Stm32GpioPin>(portDevice_->OpenPin(static_cast<PortPins>(index))), access);
	}
private:
	ObjectAccessor<PortDevice> portDevice_;
};

GpioDriver::GpioDriver(const FDTDevice& device)
	:device_(device)
{

}

void GpioDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32GpioController>(device_));
}
