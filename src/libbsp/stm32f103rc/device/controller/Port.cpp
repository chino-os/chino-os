//
// Kernel Device
//
#include "Port.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <string>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(PortDriver, "port", "st,stm32f103-port");

PortPin::PortPin(ObjectAccessor<PortDevice>&& port, PortPins pin)
	:port_(std::move(port)), pin_(pin)
{
}

void PortPin::SetMode(PortInputMode mode)
{

}

void PortPin::SetMode(PortOutputMode mode, PortSpeed speed)
{

}

PortDriver::PortDriver(const FDTDevice& device)
	:device_(device)
{

}

void PortDriver::Install()
{
	g_DeviceMgr->InstallDevice(*MakeObject<PortDevice>(device_));
}

PortDevice::PortDevice(const FDTDevice & fdt)
	:usedPins_(0)
{
	auto regProp = fdt.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);

	g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
}

ObjectAccessor<PortPin> PortDevice::OpenPin(PortPins pin)
{
	ValidateExclusiveUsePin(pin);
	ObjectAccessContext context;
	context.AccessAcquired = OA_Read | OA_Write;
	Open(context);

	ObjectAccessContext pinContext;
	pinContext.AccessAcquired = OA_Read | OA_Write;
	auto pinObj = MakeObject<PortPin>(ObjectAccessor<PortDevice>(std::move(context), this), pin);
	pinObj->Open(pinContext);
	MarkPinUsed(pin, true);
	return { std::move(pinContext), std::move(pinObj) };
}

void PortDevice::ClosePin(PortPins pin) noexcept
{
	MarkPinUsed(pin, false);
}

void PortDevice::MarkPinUsed(PortPins pin, bool used) noexcept
{
	if (used)
		usedPins_ |= static_cast<uint32_t>(pin);
	else
		usedPins_ &= ~static_cast<uint32_t>(pin);
}

void PortDevice::ValidateExclusiveUsePin(PortPins pin)
{
	kassert((usedPins_ & static_cast<uint32_t>(pin)) == 0);
}

void PortPin::OnLastClose()
{
	port_->ClosePin(pin_);
}
