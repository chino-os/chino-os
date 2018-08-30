//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>
#include <libdriver/devicetree/arm/cortex-m3/controller/Nvic.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Rcc.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Port.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Fsmc.hpp>
#include <libdriver/devicetree/st/stm32f10x/controller/Dmac.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Gpio.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Usart.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/I2c.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Spi.hpp>
#include <libdriver/devicetree/st/stm32f10x/io/Sdio.hpp>
#include <libdriver/devicetree/st/stm32f10x/display/lcd/lcd8080fsmc.hpp>

#include <libdriver/devicetree/atmel/storage/eeprom/at24c02.hpp>
#include <libdriver/devicetree/adi/sensor/adxl345.hpp>
#include <libdriver/devicetree/ilitek/display/lcd/ili9486.hpp>
#include <libdriver/devicetree/gd/storage/flash/gd25q128.hpp>
#include <libdriver/devicetree/microchip/network/ethernet/enc28j60.hpp>
#include <libdriver/devicetree/davicom/network/ethernet/dm9051.hpp>
#include <libdriver/devicetree/vlsi/audio/adapter/vs1053b.hpp>

using namespace Chino::Device;

extern const uint8_t _binary_devicetree_dtb_start[];
extern const uint8_t _binary_devicetree_dtb_end[];

gsl::span<const uint8_t> Chino::Device::BSPGetFdtData() noexcept
{
	return { _binary_devicetree_dtb_start , _binary_devicetree_dtb_end };
}

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	REF_FDT_DRIVER_DESC(NvicDriver),
	REF_FDT_DRIVER_DESC(RccDriver),
	REF_FDT_DRIVER_DESC(PortDriver),
	REF_FDT_DRIVER_DESC(FsmcDriver),
	REF_FDT_DRIVER_DESC(DmacDriver),
	REF_FDT_DRIVER_DESC(GpioDriver),
	REF_FDT_DRIVER_DESC(UsartDriver),
	REF_FDT_DRIVER_DESC(I2cDriver),
	REF_FDT_DRIVER_DESC(SpiDriver),
	REF_FDT_DRIVER_DESC(SdioDriver),
	REF_FDT_DRIVER_DESC(LCD8080FsmcDriver),

	REF_FDT_DRIVER_DESC(AT24C02Driver),
	REF_FDT_DRIVER_DESC(ADXL345Driver),
	REF_FDT_DRIVER_DESC(ILI9486Driver),
	REF_FDT_DRIVER_DESC(GD25Q128Driver),
	REF_FDT_DRIVER_DESC(ENC28J60Driver),
	REF_FDT_DRIVER_DESC(DM9051Driver),
	REF_FDT_DRIVER_DESC(VS1053BDriver),
	nullptr
};
