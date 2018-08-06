//
// Kernel Device
//
#include "lcd8080fsmc.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "../../controller/Fsmc.hpp"
#include <string>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Graphics;

DEFINE_FDT_DRIVER_DESC_1(LCD8080FsmcDriver, "lcd8080-driver", "st,stm32f103-lcd8080-driver-fsmc");

typedef volatile struct
{
	uint16_t TFT_CMD;
	uint16_t TFT_DATA;
} TFT_TypeDef;
#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x000007FE))
#define LCD             ((TFT_TypeDef *) LCD_BASE)

void LCD_WriteCmd(uint16_t cmd)
{
	LCD->TFT_CMD = cmd;
}

void LCD_WriteData(uint16_t dat)
{
	LCD->TFT_DATA = dat;
}

uint16_t LCD_ReadData()
{
	return LCD->TFT_DATA;
}

class LCD8080ControllerFsmc : public LCD8080Controller, public ExclusiveObjectAccess
{
public:
	LCD8080ControllerFsmc(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		fsmc_ = g_ObjectMgr->GetDirectory(WKD_Device).Open("fsmc1", access).MoveAs<FsmcController>();

		NorSramTiming rtim, wtim;
		rtim.AddressSetupTime = 0x01;
		rtim.AddressHoldTime = 0x00;
		rtim.DataSetupTime = 0x0F;

		wtim.AddressSetupTime = 0x00;
		wtim.AddressHoldTime = 0x00;
		wtim.DataSetupTime = 0x03;

		fsmc_->ConfigureSram(FsmcBank1Sectors::Sram4, rtim, wtim);
	}

	virtual void OnLastClose() override
	{
		fsmc_.Reset();
	}

	virtual void WriteRead(uint16_t reg, BufferList<const uint16_t> writeBufferList, BufferList<uint16_t> readBufferList) override
	{
		LCD_WriteCmd(reg);

		for (auto& buffer : writeBufferList.Buffers)
		{
			for (auto data : buffer)
				LCD_WriteData(data);
		}

		for (auto& buffer : readBufferList.Buffers)
		{
			for (auto& data : buffer)
				data = LCD_ReadData();
		}
	}

	virtual void Write(uint16_t reg, SurfaceData& surface) override
	{
		gsl::span<const uint16_t> span = { reinterpret_cast<const uint16_t*>(surface.Data.data()), surface.Data.size() / 2 };

		LCD_WriteCmd(reg);

		auto src = span.data();
		for (size_t y = 0; y < surface.Rect.GetSize().Width; y++)
		{ 
			for (size_t x = 0; x < surface.Rect.GetSize().Height; x++)
				LCD_WriteData(src[x]);

			src += y * surface.Stride / 2;
		}

		for (auto data : span)
			LCD_WriteData(data);
	}

	virtual void Fill(uint16_t reg, uint16_t value, size_t count)
	{
		LCD_WriteCmd(reg);

		for (size_t i = 0; i < count; i++)
			LCD_WriteData(value);
	}
private:
	const FDTDevice& fdt_;
	ObjectAccessor<FsmcController> fsmc_;
};

LCD8080FsmcDriver::LCD8080FsmcDriver(const FDTDevice& device)
	:device_(device)
{

}

void LCD8080FsmcDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<LCD8080ControllerFsmc>(device_));
}