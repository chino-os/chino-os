//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>
#include <libdriver/devicetree/controller/Rcc.hpp>
#include <libdriver/devicetree/controller/Port.hpp>
#include <libdriver/devicetree/io/Gpio.hpp>
#include <libdriver/devicetree/io/Usart.hpp>

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
