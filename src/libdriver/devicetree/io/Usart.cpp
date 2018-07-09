//
// Kernel Device
//
#include "Usart.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Serial.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(UsartDriver, "serial", "st,stm32f103-usart");

typedef volatile struct
{
	uint16_t SR;
	uint16_t  RESERVED0;
	uint16_t DR;
	uint16_t  RESERVED1;
	uint16_t BRR;
	uint16_t  RESERVED2;
	uint16_t CR1;
	uint16_t  RESERVED3;
	uint16_t CR2;
	uint16_t  RESERVED4;
	uint16_t CR3;
	uint16_t  RESERVED5;
	uint16_t GTPR;
	uint16_t  RESERVED6;
} USART_TypeDef;

UsartDriver::UsartDriver(const FDTDevice& device)
	:device_(device)
{

}

class UsartSerial : public Serial, public ExclusiveObjectAccess
{
public:
	UsartSerial(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		usart_ = reinterpret_cast<decltype(usart_)>(regProp->GetUInt32(0));
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto port = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("port")->GetString(), access).MoveAs<PortDevice>();
		pinTx_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("tx_pin")->GetUInt32(access)));
		pinRx_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("rx_pin")->GetUInt32(access)));

		(*pinTx_)->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		(*pinRx_)->SetMode(PortInputMode::Floating);

		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::USART1, true);
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::USART1, false);
		pinTx_.reset();
		pinRx_.reset();
	}

	virtual void SetBaudRate(size_t baudRate) override
	{

	}

	virtual void SetParity(Parity parity) override
	{

	}

	virtual void SetStopBits(StopBits stopBits) override
	{

	}

	virtual void SetDataBits(size_t dataBits) override
	{

	}

	virtual size_t Read(gsl::span<uint8_t> buffer) override
	{

	}

	virtual void Write(gsl::span<const uint8_t> buffer) override
	{

	}
private:
	USART_TypeDef* usart_;
	const FDTDevice& fdt_;
	std::optional<ObjectAccessor<PortPin>> pinTx_, pinRx_;
};

void UsartDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<UsartSerial>(device_));
}
