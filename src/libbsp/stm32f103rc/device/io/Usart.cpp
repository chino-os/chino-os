//
// Kernel Device
//
#include "Usart.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Serial.hpp>
#include <kernel/object/ObjectManager.hpp>
#include "../controller/Rcc.hpp"

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(UsartDriver, "serial", "st,stm32f103-usart");

UsartDriver::UsartDriver(const FDTDevice& device)
	:device_(device)
{

}

class UsartSerial : public Serial
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

	virtual void SetIsEnabled(bool enable) override
	{
		auto rcc = g_ObjectMgr->GetDirectory(WKD_Device).Open("rcc", OA_ReadWrite).MoveAs<RccDevice>();
		rcc->SetPeriphClockIsEnabled(RccPeriph::USART1, enable);
	}

	virtual void Open() override
	{

	}

	virtual void Close() override
	{

	}
private:
	volatile USART_TypeDef* usart_;
};

void UsartDriver::Install()
{
}
