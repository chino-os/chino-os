//
// Kernel Device
//
#include "Usart.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Serial.hpp>
#include <kernel/object/ObjectManager.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(UsartDriver, "serial", "st,stm32f103-usart");

UsartDriver::UsartDriver(const FDTDevice& device)
	:device_(device)
{

}

class UsartSerial : public Serial, public ExclusiveObjectAccess
{
	typedef struct
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
public:
	UsartSerial(const FDTDevice& fdt)
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		usart_ = reinterpret_cast<decltype(usart_)>(regProp->GetUInt32(0));
	}

	virtual void Start() override
	{
		SetIsEnabled(true);
	}

	virtual void Stop() override
	{
		SetIsEnabled(false);
		portPinTx_.reset();
		portPinRx_.reset();
	}
private:
	void SetIsEnabled(bool enable)
	{
		auto rcc = g_ObjectMgr->GetDirectory(WKD_Device).Open("Rcc", OA_Read | OA_Write).MoveAs<RccDevice>();
		rcc->SetPeriphClockIsEnabled(RccPeriph::USART1, enable);
	}
private:
	volatile USART_TypeDef* usart_;
	std::optional<ObjectAccessor<PortPin>> portPinTx_, portPinRx_;
};

void UsartDriver::Install()
{
	
}
