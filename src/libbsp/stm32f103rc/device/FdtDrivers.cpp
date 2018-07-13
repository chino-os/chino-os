//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Rcc.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Port.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Gpio.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Usart.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/I2c.hpp>

using namespace Chino::Device;

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	REF_FDT_DRIVER_DESC(RccDriver),
	REF_FDT_DRIVER_DESC(PortDriver),
	REF_FDT_DRIVER_DESC(GpioDriver),
	REF_FDT_DRIVER_DESC(UsartDriver),
	REF_FDT_DRIVER_DESC(I2cDriver),
	nullptr
};
