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
public:
	virtual void SetIsEnabled(bool enable) override
	{
		auto rcc = g_ObjectMgr->GetDirectory(WellKnownDirectory::Device).GetItem("rcc").As<RccDevice>();
		
	}

	virtual void Open() override
	{

	}

	virtual void Close() override
	{

	}
};

void UsartDriver::Install()
{
	auto regProp = device_.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);
}
