//
// Kernel Device
//
#include "Spi.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Spi.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"
#include "../controller/Dmac.hpp"
#include <cmath>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(SpiDriver, "spi", "st,stm32f103-spi");

enum spi_baud_rate
{
	SPI_BD_Div2 = 0b000,			//!< fPCLK/2
	SPI_BD_Div4 = 0b001,			//!< fPCLK/4
	SPI_BD_Div8 = 0b010,			//!< fPCLK/8
	SPI_BD_Div16 = 0b011,			//!< fPCLK/16
	SPI_BD_Div32 = 0b100,			//!< fPCLK/32
	SPI_BD_Div64 = 0b101,			//!< fPCLK/64
	SPI_BD_Div128 = 0b110,			//!< fPCLK/128
	SPI_BD_Div256 = 0b111			//!< fPCLK/256
};

enum spi_dff
{
	SPI_DFF_8Bit = 0,				//!< 8-bit data frame format is selected for transmission/reception
	SPI_DFF_16Bit = 1				//!< 16-bit data frame format is selected for transmission/reception
};

enum spi_bimode
{
	SPI_BM_2LineBidir = 0,			//!< 2-line unidirectional data mode selected
	SPI_BM_1LineBidir = 1			//!< 1-line bidirectional data mode selected
};

union spi_cr1
{
	struct
	{
		uint16_t CPHA : 1;			//!< Clock phase
		uint16_t CPOL : 1;			//!< Clock polarity
		uint16_t MSTR : 1;			//!< Master selection
		spi_baud_rate BR : 3;		//!< Baud rate control
		uint16_t SPE : 1;			//!< SPI enable
		uint16_t LSBFIRST : 1;		//!< Frame format
		uint16_t SSI : 1;			//!< Internal slave select
		uint16_t SSM : 1;			//!< Software slave management
		uint16_t RXONLY : 1;		//!< Receive only
		spi_dff DFF : 1;			//!< Data frame format
		uint16_t CRCNEXT : 1;		//!< Transmit CRC next
		uint16_t CRCEN : 1;			//!< Hardware CRC calculation enable
		uint16_t BIDIOE : 1;		//!< Output enable in bidirectional mode
		spi_bimode BIDIMODE : 1;	//!< Bidirectional data mode enable
	};

	uint16_t Value;
};

union spi_cr2
{
	struct
	{
		uint32_t RXDMAEN : 1;		//!< Rx buffer DMA enable
		uint32_t TXDMAEN : 1;		//!< Tx buffer DMA enable
		uint32_t SSOE : 1;			//!< SS output enable
		uint32_t RESV0 : 2;
		uint32_t ERRIR : 1;			//!< Error interrupt enable
		uint32_t RXNEIE : 1;		//!< RX buffer not empty interrupt enable
		uint32_t TXEIE : 1;			//!< Tx buffer empty interrupt enable
		uint32_t RESV1 : 24;
	};

	uint32_t Value;
};

union spi_sr
{
	struct
	{
		uint32_t RXNE : 1;			//!< Receive buffer not empty
		uint32_t TXE : 1;			//!< Transmit buffer empty
		uint32_t RESV0 : 2;
		uint32_t MODF : 1;			//!< Mode fault
		uint32_t OVR : 1;			//!< Overrun flag
		uint32_t BSY : 1;			//!< Busy flag
		uint32_t RESV1 : 24;
	};

	uint32_t Value;
};

typedef volatile struct
{
	spi_cr1 CR1;
	spi_cr2 CR2;
	spi_sr SR;
	uint32_t DR;
	uint32_t CRCPR;
	uint32_t RXCRCR;
	uint32_t TXCRCR;
	uint32_t CFGR;
	uint32_t PR;
} SPI_TypeDef;

class Stm32SpiController;

class Stm32SpiDevice : public SpiDevice, public ExclusiveObjectAccess
{
public:
	Stm32SpiDevice(ObjectAccessor<Stm32SpiController>&& controller, ChipSelectPin& csPin, SpiMode mode, uint32_t dataBitLength)
		:controller_(std::move(controller)), csPin_(csPin), mode_(mode), dataBitLength_(dataBitLength), baudRate_(SPI_BD_Div256)
	{

	}

	virtual double SetSpeed(double speed) override;
	virtual void Write(BufferList<const uint8_t> bufferList) override;
	virtual void TransferFullDuplex(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) override;
	virtual void TransferSequential(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) override;
private:
	friend class Stm32SpiController;

	ObjectAccessor<Stm32SpiController> controller_;
	ChipSelectPin& csPin_;
	SpiMode mode_;
	uint32_t dataBitLength_;
	spi_baud_rate baudRate_;
};

class Stm32SpiController : public SpiController, public FreeObjectAccess
{
	struct CSPin : public DmaSessionHandler
	{
		ChipSelectPin& Pin;

		CSPin(ChipSelectPin& pin)
			:Pin(pin)
		{
		}

		~CSPin()
		{
			Pin.Deactivate();
		}

		virtual void OnStart() override
		{
			Pin.Activate();
		}

		virtual void OnStop() override
		{

		}
	};

	enum
	{
		DMAThreshold = 33
	};
public:
	Stm32SpiController(const FDTDevice& fdt)
		:fdt_(fdt), mutex_(MakeObject<Mutex>())
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		spi_ = reinterpret_cast<decltype(spi_)>(regProp->GetUInt32(0));
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
		periph_ = static_cast<RccPeriph>(size_t(RccPeriph::SPI1) + fdt.GetName().back() - '1');
	}

	virtual ObjectAccessor<SpiDevice> OpenDevice(uint32_t chipSelectMask, SpiMode mode, uint32_t dataBitLength, ObjectAccess access) override
	{
		throw std::runtime_error("Not supported.");
	}

	virtual ObjectAccessor<SpiDevice> OpenDevice(ChipSelectPin& csPin, SpiMode mode, uint32_t dataBitLength, ObjectAccess access) override
	{
		return MakeAccessor(MakeObject<Stm32SpiDevice>(
			MakeAccessor<Stm32SpiController>(this, OA_Read | OA_Write), csPin, mode, dataBitLength), access);
	}

	double SetSpeed(Stm32SpiDevice& device, double speed)
	{
		double clk = RccDevice::Rcc1GetClockFrequency(periph_);
		auto div = (uint32_t)std::min(8.0, std::max(1.0, std::ceil(std::log2(clk / speed))));
		speed = clk / (1 << div);
		device.baudRate_ = static_cast<spi_baud_rate>(div - 1);
		return speed;
	}

	void Write(Stm32SpiDevice& device, BufferList<const uint8_t> bufferList)
	{
		Locker<Mutex> locker(mutex_);

		{
			CSPin cs(device.csPin_);
			spi_->CR1.SPE = 1;
			SetupDevice(device);

			WriteData(device, bufferList, cs);

			while (spi_->SR.BSY);
			spi_->CR1.SPE = 0;
		}
	}

	void TransferFullDuplex(Stm32SpiDevice& device, BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList)
	{
		kassert(!"Not impl.");
	}

	void TransferSequential(Stm32SpiDevice& device, BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList)
	{
		Locker<Mutex> locker(mutex_);

		{
			CSPin cs(device.csPin_);
			spi_->CR1.SPE = 1;
			SetupDevice(device);

			WriteData(device, writeBufferList, cs);
			if (!readBufferList.IsEmpty())
				ReadData(device, readBufferList, cs);

			while (spi_->SR.BSY);
			spi_->CR1.SPE = 0;
		}
	}
protected:
	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto port = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("port")->GetString(), access).MoveAs<PortDevice>();
		//nssPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("nss_pin")->GetUInt32(0)));
		sckPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("sck_pin")->GetUInt32(0)));
		misoPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("miso_pin")->GetUInt32(0)));
		mosiPin_ = port->OpenPin(static_cast<PortPins>(fdt_.GetProperty("mosi_pin")->GetUInt32(0)));

		//nssPin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		sckPin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		misoPin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		mosiPin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);

		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, true);
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, false);
		//nssPin_.Reset();
		sckPin_.Reset();
		misoPin_.Reset();
		mosiPin_.Reset();
	}
private:
	void SetupDevice(Stm32SpiDevice& device)
	{
		spi_cr1 cr1;
		cr1.Value = spi_->CR1.Value;

		switch (device.mode_)
		{
		case SpiMode::Mode0:
			cr1.CPOL = 0;
			cr1.CPHA = 0;
			break;
		case SpiMode::Mode1:
			cr1.CPOL = 0;
			cr1.CPHA = 1;
			break;
		case SpiMode::Mode2:
			cr1.CPOL = 1;
			cr1.CPHA = 0;
			break;
		case SpiMode::Mode3:
			cr1.CPOL = 1;
			cr1.CPHA = 1;
			break;
		default:
			throw std::invalid_argument("Invalid spi mode.");
		}

		cr1.MSTR = 1;
		cr1.SSI = 1;
		cr1.SSM = 1;
		cr1.BIDIMODE = SPI_BM_2LineBidir;
		cr1.RXONLY = 0;

		switch (device.dataBitLength_)
		{
		case 8:
			cr1.DFF = SPI_DFF_8Bit;
			break;
		case 16:
			cr1.DFF = SPI_DFF_16Bit;
			break;
		default:
			throw std::invalid_argument("Invalid data bit length.");
		}

		cr1.BR = device.baudRate_;
		while (spi_->SR.BSY);
		spi_->CR1.Value = cr1.Value;
	}

	void WriteData(Stm32SpiDevice& device, BufferList<const uint8_t> bufferList, CSPin& cs)
	{
		if (bufferList.GetTotalSize() < DMAThreshold)
		{
			kernel_critical kc;
			cs.OnStart();
			if (device.dataBitLength_ == 8)
			{
				for (auto& buffer : bufferList.Buffers)
				{
					for (auto data : buffer)
					{
						while (!spi_->SR.TXE);
						spi_->DR = data;
						while (!spi_->SR.RXNE);
						auto value = spi_->DR;
					}
				}
			}
			else
			{
				for (auto& oriBuffer : bufferList.Buffers)
				{
					gsl::span<const uint16_t> buffer(reinterpret_cast<const uint16_t*>(oriBuffer.data()), oriBuffer.size() / 2);
					for (auto data : buffer)
					{
						while (!spi_->SR.TXE);
						spi_->DR = data;
						while (!spi_->SR.RXNE);
						auto value = spi_->DR;
					}
				}
			}
		}
		else
		{
			if (device.dataBitLength_ == 8)
			{
				uint8_t dummy[1];
				gsl::span<uint8_t> readDestBuffers[] = { dummy };
				gsl::span<const volatile uint8_t> readSrcBuffers[] = { { reinterpret_cast<const volatile uint8_t*>(&spi_->DR), 1 } };

				auto dmac = g_ObjectMgr->GetDirectory(WKD_Device).Open("dmac1", OA_Read | OA_Write).MoveAs<DmaController>();
				auto readDma = dmac->OpenChannel(DmaRequestLine::SPI2_RX);
				readDma->Configure<volatile uint8_t, uint8_t>(DmaTransmition::Perip2Mem, { readSrcBuffers }, { readDestBuffers }, bufferList.GetTotalSize(), &cs);

				gsl::span<volatile uint8_t> writeDestBuffers[] = { { reinterpret_cast<volatile uint8_t*>(&spi_->DR), 1 } };
				auto writeDma = dmac->OpenChannel(DmaRequestLine::SPI2_TX);
				writeDma->Configure<uint8_t, volatile uint8_t>(DmaTransmition::Mem2Periph, bufferList, { writeDestBuffers }, 0, &cs);

				auto readEvent = readDma->StartAsync();
				auto writeEvent = writeDma->StartAsync();

				spi_->CR2.RXDMAEN = 1;
				spi_->CR2.TXDMAEN = 1;

				readEvent->GetResult();
				writeEvent->GetResult();

				spi_->CR2.RXDMAEN = 0;
				spi_->CR2.TXDMAEN = 0;
			}
			else
			{
				for (auto& oriBuffer : bufferList.Buffers)
				{
					gsl::span<const uint16_t> buffer(reinterpret_cast<const uint16_t*>(oriBuffer.data()), oriBuffer.size() / 2);
					for (auto data : buffer)
					{
						while (!spi_->SR.TXE);
						spi_->DR = data;
						while (!spi_->SR.RXNE);
						auto value = spi_->DR;
					}
				}
			}
		}

		while (!spi_->SR.TXE);
	}

	void ReadData(Stm32SpiDevice& device, BufferList<uint8_t> bufferList, CSPin& cs)
	{
		if (bufferList.GetTotalSize() < DMAThreshold)
		{
			kernel_critical kc;
			cs.OnStart();
			if (device.dataBitLength_ == 8)
			{
				for (auto& buffer : bufferList.Buffers)
				{
					for (auto& data : buffer)
					{
						while (!spi_->SR.TXE);
						spi_->DR = 0xFF;
						while (!spi_->SR.RXNE);
						data = spi_->DR;
					}
				}
			}
			else
			{
				for (auto& oriBuffer : bufferList.Buffers)
				{
					gsl::span<uint16_t> buffer(reinterpret_cast<uint16_t*>(oriBuffer.data()), oriBuffer.size() / 2);
					for (auto& data : buffer)
					{
						while (!spi_->SR.TXE);
						spi_->DR = 0xFF;
						while (!spi_->SR.RXNE);
						data = spi_->DR;
					}
				}
			}
		}
		else
		{
			if (device.dataBitLength_ == 8)
			{
				uint8_t dummy[1] = { 0xFF };
				gsl::span<const uint8_t> writeSrcBuffers[] = { dummy };
				gsl::span<volatile uint8_t> writeDestBuffers[] = { { reinterpret_cast<volatile uint8_t*>(&spi_->DR), 1 } };

				auto dmac = g_ObjectMgr->GetDirectory(WKD_Device).Open("dmac1", OA_Read | OA_Write).MoveAs<DmaController>();
				auto readDma = dmac->OpenChannel(DmaRequestLine::SPI2_TX);
				readDma->Configure<uint8_t, volatile uint8_t>(DmaTransmition::Mem2Periph, { writeSrcBuffers }, { writeDestBuffers }, bufferList.GetTotalSize(), &cs);

				gsl::span<const volatile uint8_t> readSrcBuffers[] = { { reinterpret_cast<const volatile uint8_t*>(&spi_->DR), 1 } };
				auto writeDma = dmac->OpenChannel(DmaRequestLine::SPI2_RX);
				writeDma->Configure<volatile uint8_t, uint8_t>(DmaTransmition::Perip2Mem, { readSrcBuffers }, bufferList, 0, &cs);

				auto readEvent = readDma->StartAsync();
				auto writeEvent = writeDma->StartAsync();

				spi_->CR2.RXDMAEN = 1;
				spi_->CR2.TXDMAEN = 1;

				readEvent->GetResult();
				writeEvent->GetResult();

				spi_->CR2.RXDMAEN = 0;
				spi_->CR2.TXDMAEN = 0;
			}
			else
			{
				for (auto& oriBuffer : bufferList.Buffers)
				{
					gsl::span<uint16_t> buffer(reinterpret_cast<uint16_t*>(oriBuffer.data()), oriBuffer.size() / 2);
					for (auto& data : buffer)
					{
						while (!spi_->SR.RXNE);
						data = spi_->DR;
					}
				}
			}
		}
	}
private:
	const FDTDevice& fdt_;
	SPI_TypeDef* spi_;
	RccPeriph periph_;
	ObjectAccessor<PortPin> nssPin_, sckPin_, misoPin_, mosiPin_;
	ObjectPtr<Mutex> mutex_;
};

SpiDriver::SpiDriver(const FDTDevice& device)
	:device_(device)
{

}

void SpiDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32SpiController>(device_));
}

double Stm32SpiDevice::SetSpeed(double speed)
{
	return controller_->SetSpeed(*this, speed);
}

void Stm32SpiDevice::Write(BufferList<const uint8_t> bufferList)
{
	controller_->Write(*this, bufferList);
}

void Stm32SpiDevice::TransferFullDuplex(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList)
{
	controller_->TransferFullDuplex(*this, writeBufferList, readBufferList);
}

void Stm32SpiDevice::TransferSequential(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList)
{
	controller_->TransferSequential(*this, writeBufferList, readBufferList);
}
