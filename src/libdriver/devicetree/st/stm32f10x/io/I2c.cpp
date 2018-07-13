//
// Kernel Device
//
#include "I2c.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/I2c.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"
#include <kernel/threading/ThreadSynchronizer.hpp>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(I2cDriver, "i2c", "st,stm32f103-i2c");

struct i2c_cr1
{
	uint32_t PE : 1;		//!< Peripheral enable
	uint32_t SMBUS : 1;		//!< SMBus mode
	uint32_t RESV0 : 1;		//!< Reserved
	uint32_t SMBTYPE : 1;	//!< SMBus type
	uint32_t ENARP : 1;		//!< ARP enable
	uint32_t ENPEC : 1;		//!< PEC enable
	uint32_t ENGC : 1;		//!< General call enable
	uint32_t NOSTRETCH : 1;	//!< Clock stretching disable
	uint32_t START : 1;		//!< Start generation
	uint32_t STOP : 1;		//!< Stop generation
	uint32_t ACK : 1;		//!< Acknowledge enable
	uint32_t POS : 1;		//!< Acknowledge/PEC Position
	uint32_t PEC : 1;		//!< Packet error checking
	uint32_t ALERT : 1;		//!< SMBus alert
	uint32_t RESV1 : 1;		//!< Reserved
	uint32_t SWRST : 1;		//!< Software reset
	uint32_t RESV2 : 16;	//!< Reserved
};

struct i2c_cr2
{
	uint32_t FREQ : 6;		//!< Peripheral clock frequency
	uint32_t RESV0 : 2;		//!< Reserved
	uint32_t ITERREN : 1;	//!< Error interrupt enable
	uint32_t ITEVTEN : 1;	//!< Event interrupt enable
	uint32_t ITBUFEN : 1;	//!< Buffer interrupt enable
	uint32_t DMAEN : 1;		//!< DMA requests enable
	uint32_t LAST : 1;		//!< DMA last transfer
	uint32_t RESV1 : 19;	//!< Reserved
};

struct i2c_ccr
{
	uint32_t CCR : 12;		//!< Clock control register in Fm/Sm mode (Master mode)
	uint32_t RESV0 : 2;		//!< Reserved
	uint32_t DUTY : 1;		//!< Fm mode duty cycle
	uint32_t FS : 1;		//!< I2C master mode selection
	uint32_t RESV1 : 16;	//!< Reserved
};

struct i2c_sr2
{
	uint32_t MSL : 1;			//!< Master/slave
	uint32_t BUSY : 1;			//!< Bus busy
	uint32_t TRA : 1;			//!< Transmitter/receiver
	uint32_t RESV0 : 1;			//!< Reserved
	uint32_t GENCALL : 1;		//!< General call address (Slave mode)
	uint32_t SMBDEFAULT : 1;	//!< SMBus device default address (Slave mode)
	uint32_t SMBHOST : 1;		//!< SMBus host header (Slave mode)
	uint32_t DUALF : 1;			//!< Dual flag (Slave mode)
	uint32_t PEC : 8;			//!< Packet error checking register
	uint32_t RESV1 : 16;		//!< Reserved
};

typedef volatile struct
{
	i2c_cr1 CR1;
	i2c_cr2 CR2;
	uint32_t OAR1;
	uint32_t OAR2;
	uint32_t DR;
	uint32_t SR1;
	i2c_sr2 SR2;
	i2c_ccr CCR;
	uint32_t TRISE;
} I2C_TypeDef2;

class Stm32I2cController;

class Stm32I2cDevice final : public I2cDevice, public ExclusiveObjectAccess
{
public:
	Stm32I2cDevice(ObjectAccessor<Stm32I2cController>&& controller, uint32_t slaveAddress)
		:controller_(std::move(controller)), slaveAddress_(slaveAddress)
	{

	}

	virtual size_t Read(gsl::span<uint8_t> buffer) override;
	virtual void Write(gsl::span<const uint8_t> buffer) override;
private:
	friend class Stm32I2cController;

	ObjectAccessor<Stm32I2cController> controller_;
	uint32_t slaveAddress_;
};

class Stm32I2cController final : public I2cController, public FreeObjectAccess
{
public:
	Stm32I2cController(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		i2c_ = reinterpret_cast<decltype(i2c_)>(regProp->GetUInt32(0));
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
		periph_ = static_cast<RccPeriph>(size_t(RccPeriph::I2C1) + fdt.GetName().back() - '1');
	}

	virtual ObjectAccessor<I2cDevice> OpenDevice(uint32_t slaveAddress, ObjectAccess access) override
	{
		return MakeAccessor(MakeObject<Stm32I2cDevice>(
			MakeAccessor<Stm32I2cController>(this, OA_Read | OA_Write), slaveAddress), access);
	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto port = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("port")->GetString(), access).MoveAs<PortDevice>();
		sclPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("scl_pin")->GetUInt32(access)));
		sdaPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("sda_pin")->GetUInt32(access)));

		(*sclPin_)->SetMode(PortOutputMode::AF_OpenDrain, PortOutputSpeed::PS_50MHz);
		(*sdaPin_)->SetMode(PortOutputMode::AF_OpenDrain, PortOutputSpeed::PS_50MHz);

		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, true);
		i2c_->CR1.PE = 1;
		i2c_->CR2.FREQ = RccDevice::Rcc1GetClockFrequency(periph_) / 1000000;
		g_Logger->PutChar('N');
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, false);
		sdaPin_.reset();
		sclPin_.reset();
	}

	size_t Read(ObjectPtr<Stm32I2cDevice> device, gsl::span<uint8_t> buffer)
	{
		Threading::kernel_critical kc;

		auto i2c = i2c_;

		g_Logger->PutFormat("i2c: %p, freq: %d\n", i2c, i2c_->CR2.FREQ);

		i2c->CR1.PE = 0;
		i2c->CCR.CCR = 0x04;
		i2c->TRISE = RccDevice::Rcc1GetClockFrequency(periph_) / 1000000 + 1;
		i2c->CR1.PE = 1;
		i2c->CR1.ACK = 1;

		i2c->CR1.START = 1;
		while (!i2c->SR2.MSL);
		g_Logger->PutChar('B');
		i2c->DR = 0xA0;
		while (!i2c->SR2.TRA);
		g_Logger->PutChar('C');
		i2c->DR = 0x00;
		while (!i2c->SR2.TRA);
	}

	void Write(ObjectPtr<Stm32I2cDevice> device, gsl::span<const uint8_t> buffer)
	{

	}
private:
	I2C_TypeDef2 * i2c_;
	const FDTDevice& fdt_;
	RccPeriph periph_;
	std::optional<ObjectAccessor<PortPin>> sclPin_, sdaPin_;
};

size_t Stm32I2cDevice::Read(gsl::span<uint8_t> buffer)
{
	return controller_->Read(this, buffer);
}

void Stm32I2cDevice::Write(gsl::span<const uint8_t> buffer)
{
	controller_->Write(this, buffer);
}

I2cDriver::I2cDriver(const FDTDevice& device)
	:device_(device)
{

}

void I2cDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32I2cController>(device_));
}
