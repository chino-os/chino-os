//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Rcc.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Port.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Gpio.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Usart.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/I2c.hpp>

#include <libdriver/devicetree/atmel/storage/eeprom/at24c02.hpp>
#include <libdriver/devicetree/adi/sensor/adxl345.hpp>
#include <libdriver/devicetree/ilitek/display/lcd/ili9341.hpp>

using namespace Chino::Device;

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	REF_FDT_DRIVER_DESC(RccDriver),
	REF_FDT_DRIVER_DESC(PortDriver),
	REF_FDT_DRIVER_DESC(GpioDriver),
	REF_FDT_DRIVER_DESC(UsartDriver),
	REF_FDT_DRIVER_DESC(I2cDriver),

	REF_FDT_DRIVER_DESC(AT24C02Driver),
	REF_FDT_DRIVER_DESC(ADXL345Driver),
	REF_FDT_DRIVER_DESC(ILI9341Driver),
	nullptr
};
