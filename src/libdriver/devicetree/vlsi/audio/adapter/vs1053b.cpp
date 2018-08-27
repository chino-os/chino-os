//
// Kernel Device
//
#include "vs1053b.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <kernel/device/io/Spi.hpp>
#include <kernel/device/io/Gpio.hpp>
#include <kernel/device/storage/filesystem/FileSystemManager.hpp>
#include <libbsp/bsp.hpp>

using namespace Chino;
using namespace Chino::Audio;
using namespace Chino::Device;
using namespace Chino::Threading;

#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03

/* VS10XX¼Ä´æÆ÷¶¨Òå */
#define SCI_MODE        	0x00   
#define SCI_STATUS      	0x01   
#define SCI_BASS        	0x02   
#define SCI_CLOCKF      	0x03   
#define SCI_DECODE_TIME 	0x04   
#define SCI_AUDATA      	0x05   
#define SCI_WRAM        	0x06   
#define SCI_WRAMADDR    	0x07   
#define SCI_HDAT0       	0x08   
#define SCI_HDAT1       	0x09 

#define SCI_AIADDR      	0x0a   
#define SCI_VOL         	0x0b   
#define SCI_AICTRL0     	0x0c   
#define SCI_AICTRL1     	0x0d   
#define SCI_AICTRL2     	0x0e   

union sci_mode
{
	struct
	{
		uint16_t SM_DIFF : 1;
		uint16_t SM_LAYER12 : 1;
		uint16_t SM_RESET : 1;
		uint16_t SM_CANCEL : 1;
		uint16_t SM_EARSPEAKER_LO : 1;
		uint16_t SM_TESTS : 1;
		uint16_t SM_STREAM : 1;
		uint16_t SM_EARSPEAKER_HI : 1;
		uint16_t SM_DACT : 1;
		uint16_t SM_SDIORD : 1;
		uint16_t SM_SDISHARE : 1;
		uint16_t SM_SDINEW : 1;
		uint16_t SM_ADPCM : 1;
		uint16_t RESV0 : 1;
		uint16_t SM_LINE1 : 1;
		uint16_t SM_CLK_RANGE : 1;
	};

	uint16_t Value;
};

DEFINE_FDT_DRIVER_DESC_1(VS1053BDriver, "audio-adapter", "vlsi,vs1053b");

class VS1053BDevice : public Device, public ExclusiveObjectAccess
{
	struct CS : public ChipSelectPin
	{
		bool IsCmd;

		CS(ObjectAccessor<GpioPin>& csPin, ObjectAccessor<GpioPin>& dcsPin)
			:csPin_(csPin), dcsPin_(dcsPin), IsCmd(true)
		{
		}

		virtual void Activate() override
		{
			if (IsCmd)
			{
				dcsPin_->Write(GpioPinValue::High);
				csPin_->Write(GpioPinValue::Low);
			}
			else
			{
				dcsPin_->Write(GpioPinValue::Low);
			}
		}

		virtual void Deactivate() override
		{
			if (IsCmd)
			{
				csPin_->Write(GpioPinValue::High);
			}
			else
			{
				dcsPin_->Write(GpioPinValue::High);
			}
		}
	private:
		ObjectAccessor<GpioPin>& csPin_; 
		ObjectAccessor<GpioPin>& dcsPin_;
	};
public:
	VS1053BDevice(const FDTDevice& fdt)
		:fdt_(fdt), cs_(csPin_, xdcsPin_)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	void WriteMusic(gsl::span<const uint8_t> data)
	{
		auto times = data.size() / 32;
		const uint8_t* first = data.data();
		for (size_t i = 0; i < times; i++)
		{
			gsl::span<const uint8_t> buffers[] = { {first, 32} };
			WriteData({ buffers });
			first += 32;
		}
	}

	void Reset()
	{
		Write(SCI_MODE, 0x0804);
		BSPSleepMs(1);
		while (dreqPin_->Read() != GpioPinValue::High);
	}
protected:
	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto spi = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("spi")->GetString(), access).MoveAs<SpiController>();
		dev_ = spi->OpenDevice(cs_, SpiMode::Mode0, 8, access);
		dev_->SetSpeed(3000000);

		{
			auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("cs_gpio")->GetString(), access).MoveAs<GpioController>();
			csPin_ = gpio->OpenPin(fdt_.GetProperty("cs_pin")->GetUInt32(0), access);
			csPin_->SetDriveMode(GpioPinDriveMode::Output);
			csPin_->Write(GpioPinValue::High);
		}

		{
			auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("other_gpio")->GetString(), access).MoveAs<GpioController>();
			xdcsPin_ = gpio->OpenPin(fdt_.GetProperty("xdcs_pin")->GetUInt32(0), access);
			xdcsPin_->SetDriveMode(GpioPinDriveMode::Output);
			dreqPin_ = gpio->OpenPin(fdt_.GetProperty("dreq_pin")->GetUInt32(0), access);
			dreqPin_->SetDriveMode(GpioPinDriveMode::InputPullUp);
			rstPin_ = gpio->OpenPin(fdt_.GetProperty("rst_pin")->GetUInt32(0), access);
			rstPin_->SetDriveMode(GpioPinDriveMode::Output);
		}

		HardwareReset();

		Initialize();
	}

	virtual void OnLastClose() override
	{
		dev_.Reset();
		csPin_.Reset();
		xdcsPin_.Reset();
		dreqPin_.Reset();
		rstPin_.Reset();
	}
private:
	void Initialize()
	{
		sci_mode mode{ 0 };
		mode.SM_TESTS = 1;
		mode.SM_SDINEW = 1;
		Write(SCI_MODE, 0x0804);

		while (Read(SCI_CLOCKF) != 0X9800)
		{
			Write(SCI_CLOCKF, 0X9800);
			BSPSleepMs(10);
		}

		Write(SCI_BASS, 0);
		Write(SCI_VOL, 0x5555);
	}

	void MemoryTest()
	{
		Write(SCI_MODE, 0x0820);

		const uint8_t data[] = { 0x4D, 0xEA, 0x6D, 0x54, 0, 0, 0, 0 };
		gsl::span<const uint8_t> buffers[] = { data };
		WriteData({ buffers });

		BSPSleepMs(150);
		g_Logger->PutFormat("SDI MEM TEST: %x\n", Read(SCI_HDAT0));
	}

	void SineTest()
	{
		Write(SCI_MODE, 0x0820);

		const uint8_t data[] = { 0x53, 0xEF, 0x6E, 0x24, 0, 0, 0, 0 };
		gsl::span<const uint8_t> buffers[] = { data };
		WriteData({ buffers });
	}

	void Write(uint8_t addr, uint16_t data)
	{
		while (dreqPin_->Read() != GpioPinValue::High);
		cs_.IsCmd = true;

		const uint8_t buffer[] = { VS_WRITE_COMMAND, addr, uint8_t(data >> 8), uint8_t(data) };
		gsl::span<const uint8_t> buffers[] = { buffer };
		dev_->Write({ buffers });
	}

	uint16_t Read(uint8_t addr)
	{
		while (dreqPin_->Read() != GpioPinValue::High);
		cs_.IsCmd = true;

		const uint8_t writeBuffer[] = { VS_READ_COMMAND, addr };
		gsl::span<const uint8_t> writeBuffers[] = { writeBuffer };

		uint8_t readBuffer[2];
		gsl::span<uint8_t> readBuffers[] = { readBuffer };

		dev_->TransferSequential({ writeBuffers }, { readBuffers });
		return uint16_t((readBuffer[0] << 8) | readBuffer[1]);
	}

	void WriteData(BufferList<const uint8_t> bufferList)
	{
		while (dreqPin_->Read() != GpioPinValue::High);
		cs_.IsCmd = false;
		
		dev_->Write(bufferList);
	}

	void HardwareReset()
	{
		rstPin_->Write(GpioPinValue::Low);
		BSPSleepMs(20);
		xdcsPin_->Write(GpioPinValue::High);
		rstPin_->Write(GpioPinValue::High);
		while (dreqPin_->Read() != GpioPinValue::High);
		BSPSleepMs(20);
	}
private:
	const FDTDevice& fdt_;
	ObjectAccessor<SpiDevice> dev_;
	ObjectAccessor<GpioPin> csPin_, xdcsPin_, dreqPin_, rstPin_;
	CS cs_;
};

VS1053BDriver::VS1053BDriver(const FDTDevice& device)
	:device_(device)
{

}

void VS1053BDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<VS1053BDevice>(device_));
}

class VS1053BAudioClient;

class VS1053BAudioRenderClient : public IAudioRenderClient
{
public:
	VS1053BAudioRenderClient(ObjectPtr<VS1053BAudioClient> client)
		:client_(std::move(client))
	{

	}

	virtual void GetBuffer(gsl::span<uint8_t>& buffer) override
	{
		buffer = buffer_;
	}

	virtual void ReleaseBuffer() override;
private:
	ObjectPtr<VS1053BAudioClient> client_;
	std::array<uint8_t, 32> buffer_;
};

class VS1053BAudioClient : public IAudioClient
{
public:
	VS1053BAudioClient(ObjectAccessor<VS1053BDevice>&& device)
		:device_(std::move(device))
	{

	}

	virtual bool IsFormatSupported(const AudioFormat& format) override
	{
		return format.Tag == AudioFormatTag::AutoDetect;
	}

	virtual void SetFormat(const AudioFormat& format) override
	{
		if (format.Tag != AudioFormatTag::AutoDetect)
			throw std::invalid_argument("Not supported audio format.");
	}

	virtual void Start() override
	{
		device_->Reset();
	}

	virtual void Stop() override
	{
		device_->Reset();
	}

	virtual ObjectPtr<IAudioRenderClient> GetRenderClient()
	{
		return MakeObject<VS1053BAudioRenderClient>(this);
	}

	void WriteMusic(gsl::span<const uint8_t> data)
	{
		device_->WriteMusic(data);
	}
private:
	ObjectAccessor<VS1053BDevice> device_;
};

void VS1053BAudioRenderClient::ReleaseBuffer()
{
	client_->WriteMusic(buffer_);
}

ObjectPtr<IAudioClient> Chino::Device::CreateVS1053BAudioClient(ObjectAccessor<Chino::Device::Device>&& device)
{
	return MakeObject<VS1053BAudioClient>(device.MoveAs<VS1053BDevice>());
}
