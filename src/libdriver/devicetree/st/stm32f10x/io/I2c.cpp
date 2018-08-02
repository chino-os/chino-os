//
// Kernel Device
//
#include "I2c.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/I2c.hpp>
#include <kernel/device/controller/Pic.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"
#include "../controller/Fsmc.hpp"
#include "../controller/Dmac.hpp"
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <libbsp/bsp.hpp>
#include <kernel/device/Async.hpp>
#include <kernel/memory/MemoryManager.hpp>

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

union i2c_sr2
{
	struct
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

	uint32_t Value;
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
} I2C_TypeDef;

#define  I2C_EVENT_MASTER_MODE_SELECT                      ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED        ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED                 ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */
#define  I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED           ((uint32_t)0x00030002)  /* BUSY, MSL and ADDR flags */
#define  I2C_EVENT_MASTER_BYTE_RECEIVED                    ((uint32_t)0x00030040)  /* BUSY, MSL and RXNE flags */

class Stm32I2cController;

class Stm32I2cDevice final : public I2cDevice, public ExclusiveObjectAccess
{
public:
	Stm32I2cDevice(ObjectAccessor<Stm32I2cController>&& controller, uint32_t slaveAddress)
		:controller_(std::move(controller)), slaveAddress_(slaveAddress)
	{

	}

	virtual size_t WriteRead(BufferList<const uint8_t> writeBuffer, BufferList<uint8_t> readBuffer) override;
	virtual void Write(BufferList<const uint8_t> buffer) override;
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
		FsmcSuppress fs;
		auto access = OA_Read | OA_Write;
		auto port = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("port")->GetString(), access).MoveAs<PortDevice>();
		sclPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("scl_pin")->GetUInt32(0)));
		sdaPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("sda_pin")->GetUInt32(0)));

		sclPin_->SetMode(PortOutputMode::AF_OpenDrain, PortOutputSpeed::PS_50MHz);
		sdaPin_->SetMode(PortOutputMode::AF_OpenDrain, PortOutputSpeed::PS_50MHz);

		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, true);
		i2c_->CR1.PE = 1;
		i2c_->CR2.FREQ = RccDevice::Rcc1GetClockFrequency(periph_) / 1000000;
		SetIRQEnable(true);
	}

	virtual void OnLastClose() override
	{
		SetIRQEnable(false);
		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, false);
		sdaPin_.Reset();
		sclPin_.Reset();
	}

	size_t WriteRead(ObjectPtr<Stm32I2cDevice> device, BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList)
	{
		FsmcSuppress fs;
		//Threading::kernel_critical kc;

		SetupDevice(*device);
		Start(*device, true);
		WriteData(writeBufferList);
		Start(*device, false);
		auto ret = ReadData(readBufferList);
		while (i2c_->SR2.BUSY);
		return ret;
	}

	void Write(ObjectPtr<Stm32I2cDevice> device, BufferList<const uint8_t> bufferList)
	{
		FsmcSuppress fs;
		//Threading::kernel_critical kc;

		SetupDevice(*device);
		Start(*device, true);
		WriteData(bufferList);
		Stop();
		while (i2c_->SR2.BUSY);
	}
private:
	bool CheckEvent(uint32_t event)
	{
		auto flag1 = i2c_->SR1 & 0xFFFF;
		auto flag2 = i2c_->SR2.Value & 0xFFFF;
		uint32_t status = flag1 | (flag2 << 16);
		return (status & event) == event;
	}

	void SetIRQEnable(bool enable)
	{
		auto erIrq = fdt_.GetProperty("er_irq")->GetUInt32(0);
		auto nvic = g_ObjectMgr->GetDirectory(WKD_Device).Open("nvic1", OA_Read | OA_Write).MoveAs<PicDevice>();

		if (enable)
		{
			erIrq_ = g_DeviceMgr->InstallIRQHandler(nvic->GetId(), erIrq, std::bind(&Stm32I2cController::OnErrorIRQ, this));

			nvic->SetIRQEnabled(erIrq, true);
		}
		else
		{
			nvic->SetIRQEnabled(erIrq, false);

			erIrq_.Reset();
		}
	}

	void SetupDevice(Stm32I2cDevice& device)
	{
		auto i2c = i2c_;

		while (i2c->SR2.BUSY);
		auto clk = RccDevice::Rcc1GetClockFrequency(periph_);
		size_t speed = 100000;
		auto ccr = std::max(size_t(0x04), clk / (speed << 1));

		i2c->CR1.PE = 0;
		i2c->CCR.CCR = ccr;
		i2c->TRISE = clk / 1000000 + 1;
		i2c->CR1.PE = 1;
		i2c->CR1.ACK = 1;
		i2c->CR2.ITERREN = 1;
		i2c->CR2.DMAEN = 1;
	}

	void WriteData(BufferList<const uint8_t> bufferList)
	{
		auto i2c = i2c_;

		gsl::span<volatile uint8_t> destBuffers[] = { { reinterpret_cast<volatile uint8_t*>(&i2c->DR), 1 } };
		auto dmac = g_ObjectMgr->GetDirectory(WKD_Device).Open("dmac1", OA_Read | OA_Write).MoveAs<DmaController>();
		auto dma = dmac->OpenChannel(DmaRequestLine::I2C1_TX);
		dma->Configure<uint8_t, volatile uint8_t>(DmaTransmition::Mem2Periph, bufferList, { destBuffers });
		auto event = dma->StartAsync();
		event->GetResult();
	}

	size_t ReadData(BufferList<uint8_t> bufferList)
	{
		auto i2c = i2c_;
		auto toRead = bufferList.GetTotalSize();
		auto read = 0;

		for (auto& buffer : bufferList.Buffers)
		{
			for (auto& data : buffer)
			{
				if (toRead-- == 1)
					i2c->CR1.ACK = 0;
				while (!CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));
				data = static_cast<uint8_t>(i2c->DR);
				read++;
			}
		}

		Stop();
		i2c->CR1.ACK = 1;
		return read;
	}

	void Start(Stm32I2cDevice& device, bool send)
	{
		auto i2c = i2c_;
		auto addr = device.slaveAddress_;

		i2c->CR1.START = 1;
		while (!CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
		if (send)
		{
			i2c->DR = addr << 1;
			while (!CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
		}
		else
		{
			i2c->DR = (addr << 1) | 1;
			while (!CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
		}
	}

	void Stop()
	{
		auto i2c = i2c_;
		i2c->CR1.STOP = 1;
		while (i2c->SR2.BUSY);
	}

	void OnErrorIRQ()
	{
		g_Logger->PutChar('E');
	}
private:
	I2C_TypeDef * i2c_;
	const FDTDevice& fdt_;
	RccPeriph periph_;
	ObjectAccessor<PortPin> sclPin_, sdaPin_;
	ObjectPtr<IObject> erIrq_;
	ObjectPtr<AsyncActionCompletionEvent> currentCompletionEvent_;
};

size_t Stm32I2cDevice::WriteRead(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList)
{
	return controller_->WriteRead(this, writeBufferList, readBufferList);
}

void Stm32I2cDevice::Write(BufferList<const uint8_t> buffer)
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
