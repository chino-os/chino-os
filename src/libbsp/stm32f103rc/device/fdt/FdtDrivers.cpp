//
// Kernel Device
//
#include "Fdt.hpp"
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"
#include "../io/Gpio.hpp"
#include "../io/Usart.hpp"

using namespace Chino::Device;

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	REF_FDT_DRIVER_DESC(RccDriver),
	REF_FDT_DRIVER_DESC(PortDriver),
	REF_FDT_DRIVER_DESC(GpioDriver),
	REF_FDT_DRIVER_DESC(UsartDriver),
	nullptr
};
