//
// Kernel Device
//
#include "Port.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "Rcc.hpp"
#include <string>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(PortDriver, "port", "st,stm32f103-port");

enum port_mode
{
	PM_Input = 0b00,		//!< Input mode (reset state)
	PM_Output10MHz = 0b01,	//!< Output mode, max speed 10 MHz
	PM_Output2MHz = 0b10,	//!< Output mode, max speed 2 MHz
	PM_Output50MHz = 0b11	//!< Output mode, max speed 50 MHz
};

enum port_cfg
{
	PC_Analog = 0b00,
	PC_FloatingInput = 0b01,
	PC_InputPullUpDown = 0b10,
	PC_OutputPushPull = 0b00,
	PC_OutputOpenDrain = 0b01,
	PC_AFOutputPushPull = 0b10,
	PC_AFOutputOpenDrain = 0b11
};

struct port_cr
{
	uint32_t Value;

	void Set(uint32_t pinOffset, port_mode mode, port_cfg cfg) volatile
	{
		auto value = Value & ~(0b1111 << pinOffset);
		Value = value | (mode << pinOffset) | (cfg << (pinOffset + 2));
	}
};

struct port_idr
{
	uint32_t Value;

	uint32_t Get(PortPins pin) volatile
	{
		auto idx = static_cast<uint32_t>(pin);
		return (Value & (1 << idx)) >> idx;
	}
};

typedef volatile struct
{
	port_cr CRL;		//!< Port configuration register low
	port_cr CRH;		//!< Port configuration register high
	port_idr IDR;		//!< Port input data register
} Port_TypeDef;

#define port reinterpret_cast<Port_TypeDef*>(regAddr_)

PortDriver::PortDriver(const FDTDevice& device)
	:device_(device)
{

}

void PortDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<PortDevice>(device_));
}

PortDevice::PortDevice(const FDTDevice & fdt)
	:usedPins_(0)
{
	auto regProp = fdt.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);
	portIdx_ = fdt.GetName().back() - 'A';

	g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
}

ObjectAccessor<PortPin> PortDevice::OpenPin(PortPins pin)
{
	ValidateExclusiveUsePin(pin);
	auto pinObj = MakeObject<PortPin>(MakeAccessor<PortDevice>(this, OA_Read | OA_Write), pin);
	auto accessor = MakeAccessor(pinObj, OA_Read | OA_Write);
	MarkPinUsed(pin, true);
	return std::move(accessor);
}

void PortDevice::ClosePin(PortPins pin) noexcept
{
	MarkPinUsed(pin, false);
}

void PortDevice::MarkPinUsed(PortPins pin, bool used) noexcept
{
	if (used)
		usedPins_ |= 1 << static_cast<uint32_t>(pin);
	else
		usedPins_ &= ~(1 << static_cast<uint32_t>(pin));
}

void PortDevice::ValidateExclusiveUsePin(PortPins pin)
{
	kassert((usedPins_ & static_cast<uint32_t>(pin)) == 0);
}

#define RCC_PERIPH static_cast<RccPeriph>(static_cast<uint32_t>(RccPeriph::PortA) + portIdx_)

void PortDevice::OnFirstOpen()
{
	RccDevice::Rcc1SetPeriphClockIsEnabled(RCC_PERIPH, true);
}

void PortDevice::OnLastClose()
{
	RccDevice::Rcc1SetPeriphClockIsEnabled(RCC_PERIPH, false);
}

void PortDevice::SetMode(PortPins pin, PortInputMode mode)
{
	auto offset = static_cast<uint32_t>(pin);
	auto useLow = offset < 8;
	offset = useLow ? offset : offset - 8;

	port_cfg cfg;
	switch (mode)
	{
	case PortInputMode::Analog:
		cfg = PC_Analog;
		break;
	case PortInputMode::Floating:
		cfg = PC_FloatingInput;
		break;
	case PortInputMode::PullDown:
		cfg = PC_InputPullUpDown;
		break;
	case PortInputMode::PullUp:
		cfg = PC_InputPullUpDown;
		break;
	default:
		kassert(!"Invalid port mode.");
		break;
	}

	if (useLow)
		port->CRL.Set(offset, PM_Input, cfg);
	else
		port->CRH.Set(offset, PM_Input, cfg);
}

void PortDevice::SetMode(PortPins pin, PortOutputMode mode, PortOutputSpeed speed)
{
	auto offset = static_cast<uint32_t>(pin);
	auto useLow = offset < 8;
	offset = useLow ? offset : offset - 8;

	port_cfg cfg;
	switch (mode)
	{
	case PortOutputMode::PushPull:
		cfg = PC_OutputPushPull;
		break;
	case PortOutputMode::OpenDrain:
		cfg = PC_OutputOpenDrain;
		break;
	case PortOutputMode::AF_PushPull:
		cfg = PC_AFOutputPushPull;
		break;
	case PortOutputMode::AF_OpenDrain:
		cfg = PC_AFOutputOpenDrain;
		break;
	default:
		kassert(!"Invalid port mode.");
		break;
	}

	auto pmode = static_cast<port_mode>(speed);

	if (useLow)
		port->CRL.Set(offset, pmode, cfg);
	else
		port->CRH.Set(offset, pmode, cfg);
}

PortPin::PortPin(ObjectAccessor<PortDevice>&& portDevice, PortPins pin)
	:portDevice_(std::move(portDevice)), pin_(pin)
{
}

void PortPin::SetMode(PortInputMode mode)
{
	portDevice_->SetMode(pin_, mode);
}

void PortPin::SetMode(PortOutputMode mode, PortOutputSpeed speed)
{
	portDevice_->SetMode(pin_, mode, speed);
}

void PortPin::OnLastClose()
{
	portDevice_->ClosePin(pin_);
}
