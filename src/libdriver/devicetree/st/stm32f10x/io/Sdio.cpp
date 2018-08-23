//
// Kernel Device
//
#include "Sdio.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Sdio.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include "../controller/Rcc.hpp"
#include "../controller/Port.hpp"
#include "../controller/Dmac.hpp"
#include <libbsp/bsp.hpp>
#include <cmath>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(SdioDriver, "sdio", "st,stm32f103-sdio");

enum sdio_pwrctrl
{
	SDIO_PWR_OFF = 0b00,
	SDIO_PWR_ON = 0b11
};

union sdio_power
{
	struct
	{
		sdio_pwrctrl PWRCTRL : 2;		//!< Power supply control bits
		uint32_t RESV0 : 30;
	};

	uint32_t Value;
};

enum sdio_widbus
{
	SDIO_DBW_1 = 0b00,
	SDIO_DBW_4 = 0b01,
	SDIO_DBW_8 = 0b11
};

union sdio_clkcr
{
	struct
	{
		uint32_t CLKDIV : 8;			//!< Clock divide factor
		uint32_t CLKEN : 1;				//!< Clock enable bit
		uint32_t PWRSAV : 1;			//!< Power saving configuration bit
		uint32_t BYPASS : 1;			//!< Clock divider bypass enable bit
		sdio_widbus WIDBUS : 2;			//!< Wide bus mode enable bit
		uint32_t NEGEDGE : 1;			//!< SDIO_CK dephasing selection bit
		uint32_t HWFC_EN : 1;			//!< HW Flow Control enable
		uint32_t RESV1 : 17;
	};

	uint32_t Value;
};

enum sdio_waitresp
{
	SDIO_WAITRESP_NONE = 0b00,
	SDIO_WAITRESP_SHORT = 0b01,
	SDIO_WAITRESP_LONG = 0b11
};

union sdio_cmd
{
	struct
	{
		uint32_t CMDINDEX : 6;			//!< Command index
		sdio_waitresp WAITRESP : 2;		//!< Wait for response bits
		uint32_t WAITINT : 1;			//!< CPSM waits for interrupt request
		uint32_t WAITPEND : 1;			//!< CPSM Waits for ends of data transfer(CmdPend internal signal)
		uint32_t CPSMEN : 1;			//!< Command path state machine(CPSM) Enable bit
		uint32_t SDIOSuspend : 1;		//!< SD I/O suspend command
		uint32_t ENCMDcompl : 1;		//!< Enable CMD completion
		uint32_t nIEN : 1;				//!< not interrupt enable
		uint32_t ATACMD : 1;			//!< CE-ATA command
		uint32_t RESV0 : 17;
	};

	uint32_t Value;
};

union sdio_sta
{
	struct
	{
		uint32_t CCRCFAIL : 1;			//!< Command response received
		uint32_t DCRCFAIL : 1;			//!< Data block sent/received
		uint32_t CTIMEOUT : 1;			//!< Command response timeout
		uint32_t DTIMEOUT : 1;			//!< Data timeout
		uint32_t TXUNDERR : 1;			//!< Transmit FIFO underrun error
		uint32_t RXOVERR : 1;			//!< Received FIFO overrun error
		uint32_t CMDREND : 1;			//!< Command response
		uint32_t CMDSENT : 1;			//!< Command sent (no response required)
		uint32_t DATAEND : 1;			//!< Data end (data counter, SDIDCOUNT, is zero)
		uint32_t STBITERR : 1;			//!< Start bit not detected on all data signals in wide bus mode
		uint32_t DBCKEND : 1;			//!< Data block sent/received (CRC check passed)
		uint32_t CMDACT : 1;			//!< Command transfer in progress
		uint32_t TXACT : 1;				//!< Data transmit in progress
		uint32_t RXACT : 1;				//!< Data receive in progress
		uint32_t TXFIFOHE : 1;			//!< Transmit FIFO half empty
		uint32_t RXFIFOHF : 1;			//!< Receive FIFO half full
		uint32_t TXFIFOF : 1;			//!< Transmit FIFO full
		uint32_t RXFIFOF : 1;			//!< Receive FIFO full
		uint32_t TXFIFOE : 1;			//!< Transmit FIFO empty
		uint32_t RXFIFOE : 1;			//!< Receive FIFO empty
		uint32_t TXDVAL : 1;			//!< Data available in transmit FIFO
		uint32_t RXDVAL : 1;			//!< Data available in receive FIFO
		uint32_t SDIOIT : 1;			//!< SDIO interrupt received
		uint32_t CEATAEND : 1;			//!< CE-ATA command completion signal received for CMD61
		uint32_t RESV0 : 8;
	};

	uint32_t Value;
};

union sdio_icr
{

};

enum sdio_DTDIR
{
	SD_DTDIR_Con2Card = 0,
	SD_DTDIR_Card2Con = 1
};

enum sdio_DTMODE
{
	SD_DTMODE_Block = 0,
	SD_DTMODE_Stream = 1
};

enum sdio_dblksize
{
	SD_DBLK_1 = 0b0000,
	SD_DBLK_2 = 0b0001,
	SD_DBLK_4 = 0b0010,
	SD_DBLK_8 = 0b0011,
	SD_DBLK_16 = 0b0100,
	SD_DBLK_32 = 0b0101,
	SD_DBLK_64 = 0b0110,
	SD_DBLK_128 = 0b0111,
	SD_DBLK_256 = 0b1000,
	SD_DBLK_512 = 0b1001,
	SD_DBLK_1024 = 0b1010,
	SD_DBLK_2048 = 0b1011,
	SD_DBLK_4096 = 0b1100,
	SD_DBLK_8192 = 0b1101,
	SD_DBLK_16384 = 0b1110
};

union sdio_dctrl
{
	struct
	{
		uint32_t DTEN : 1;				//!< Data transfer enabled bit
		sdio_DTDIR DTDIR : 1;			//!< Data transfer direction selection
		sdio_DTMODE DTMODE : 1;			//!< Data transfer mode selection
		uint32_t DMAEN : 1;				//!< DMA enable bit
		sdio_dblksize DBLOCKSIZE : 4;	//!< Data block size
		uint32_t RWSTART : 1;			//!< Read wait start
		uint32_t RWSTOP : 1;			//!< Read wait stop
		uint32_t RWMOD : 1;				//!< Read wait mode
		uint32_t SDIOEN : 1;			//!< SD I/O enable functions
		uint32_t RESV0 : 20;
	};

	uint32_t Value;
};

typedef volatile struct
{
	sdio_power POWER;
	sdio_clkcr CLKCR;
	uint32_t ARG;
	sdio_cmd CMD;
	uint32_t RESPCMD;
	uint32_t RESP[4];
	uint32_t DTIMER;
	uint32_t DLEN;
	sdio_dctrl DCTRL;
	uint32_t DCOUNT;
	sdio_sta STA;
	uint32_t ICR;
	uint32_t MASK;
	uint32_t RESV0[2];
	uint32_t FIFOCNT;
	uint32_t RESV1[13];
	uint32_t FIFO[32];
} SDIO_TypeDef;

#define SD_DATATIMEOUT 0xFFFFFFFF

class Stm32SdioController : public SdioController, public ExclusiveObjectAccess
{
public:
	Stm32SdioController(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		sdio_ = reinterpret_cast<decltype(sdio_)>(regProp->GetUInt32(0));
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void Reset() override
	{
		sdio_clkcr clk{ 0 };
		clk.CLKDIV = 178;
		sdio_->CLKCR.Value = clk.Value;
		sdio_->POWER.Value = 3;
		sdio_->CLKCR.CLKEN = 1;
		BSPSleepMs(10);
	}

	virtual void SetDatabusWidth(SdioDatabusWidth width) override
	{
		sdio_widbus widbus;

		switch (width)
		{
		case SdioDatabusWidth::One:
			widbus = SDIO_DBW_1;
			break;
		case SdioDatabusWidth::Four:
			widbus = SDIO_DBW_4;
			break;
		default:
			throw std::invalid_argument("Invalid sdio databus width.");
		}

		if (sdio_->CLKCR.WIDBUS != widbus)
		{
			sdio_->CLKCR.WIDBUS = widbus;
			BSPSleepMs(10);
		}
	}

	virtual void SendCommand(const SdioCommand& command) override
	{
		sdio_->ICR = 0xFFFFFFFF;

		sdio_cmd cmd{ 0 };
		cmd.CMDINDEX = static_cast<uint32_t>(command.CommandIndex);
		cmd.CPSMEN = 1;
		switch (command.ResponseType)
		{
		case SdioResponseType::None:
			cmd.WAITRESP = SDIO_WAITRESP_NONE;
			break;
		case SdioResponseType::Short:
			cmd.WAITRESP = SDIO_WAITRESP_SHORT;
			break;
		case SdioResponseType::Long:
			cmd.WAITRESP = SDIO_WAITRESP_LONG;
			break;
		default:
			throw std::invalid_argument("Invalid response type.");
		}

		sdio_->ARG = command.Argument;
		sdio_->CMD.Value = cmd.Value;

		if (command.ResponseType == SdioResponseType::None)
			while (!sdio_->STA.CMDSENT);
	}

	virtual void SendCommand(const SdioCommand& command, SdioResponse& response) override
	{
		SendCommand(command);

		if (command.ResponseType != SdioResponseType::None)
		{
			bool needCRC = command.ResponseFormat != SdioResponseFormat::R3;
			while (true)
			{
				sdio_sta sta;
				sta.Value = sdio_->STA.Value;
				if (sta.CTIMEOUT)
					throw std::runtime_error("SDIO Timeout.");
				else if (sta.CCRCFAIL)
				{
					if (needCRC)
						throw std::runtime_error("CRC Failed.");
					else
						break;
				}
				else if (sta.CMDREND)
					break;
			}

			if (command.ResponseType == SdioResponseType::Short)
				response.Data[0] = sdio_->RESP[0];
			else if (command.ResponseType == SdioResponseType::Long)
			{
				response.Data[0] = sdio_->RESP[0];
				response.Data[1] = sdio_->RESP[1];
				response.Data[2] = sdio_->RESP[2];
				response.Data[3] = sdio_->RESP[3];
			}
		}
	}

	virtual void ReadDataBlocks(const SdioCommand& command, size_t blockSize, size_t blocksCount, BufferList<uint8_t> bufferList) override
	{
		sdio_dctrl dctrl{0};
		SetBlockSize(dctrl, blockSize);
		dctrl.DTDIR = SD_DTDIR_Card2Con;
		dctrl.DTMODE = SD_DTMODE_Block;
		dctrl.DTEN = 1;
		dctrl.SDIOEN = 1;

		sdio_->DTIMER = SD_DATATIMEOUT;
		sdio_->DLEN = blockSize * blocksCount;
		sdio_->DCTRL.Value = dctrl.Value;

		SdioResponse resp;
		SendCommand(command, resp);

		kernel_critical kc;

		for (auto& buffer : bufferList.Buffers)
		{
			gsl::span<uint32_t> dest = { reinterpret_cast<uint32_t*>(buffer.data()), ptrdiff_t(buffer.size() / sizeof(uint32_t)) };
			for (auto& data : dest)
			{
				while (true)
				{
					sdio_sta sta;
					sta.Value = sdio_->STA.Value;
					if (sta.CTIMEOUT)
						throw std::runtime_error("SDIO Timeout.");
					else if (sta.CCRCFAIL)
						throw std::runtime_error("CRC Failed.");
					else if (sta.RXOVERR)
						throw std::runtime_error("Receive Overflow.");
					else if (sta.RXDVAL)
					{
						data = sdio_->FIFO[0];
						break;
					}
				}
			}
		}
	}

	virtual void WriteDataBlocks(const SdioCommand& command, size_t blockSize, size_t blocksCount, BufferList<const uint8_t> bufferList) override
	{
		sdio_dctrl dctrl{ 0 };
		SetBlockSize(dctrl, blockSize);
		dctrl.DTDIR = SD_DTDIR_Con2Card;
		dctrl.DTMODE = SD_DTMODE_Block;
		dctrl.DTEN = 1;
		dctrl.SDIOEN = 1;

		sdio_->DTIMER = SD_DATATIMEOUT;
		sdio_->DLEN = blockSize * blocksCount;
		sdio_->DCTRL.Value = dctrl.Value;

		SdioResponse resp;
		SendCommand(command, resp);

		kernel_critical kc;

		for (auto& buffer : bufferList.Buffers)
		{
			gsl::span<const uint32_t> src = { reinterpret_cast<const uint32_t*>(buffer.data()), ptrdiff_t(buffer.size() / sizeof(uint32_t)) };
			for (auto data : src)
			{
				while (true)
				{
					sdio_sta sta;
					sta.Value = sdio_->STA.Value;
					if (sta.CTIMEOUT)
						throw std::runtime_error("SDIO Timeout.");
					else if (sta.CCRCFAIL)
						throw std::runtime_error("CRC Failed.");
					else if (sta.TXUNDERR)
						throw std::runtime_error("Transmit Underflow.");
					else if (sta.TXFIFOE)
					{
						sdio_->FIFO[0] = data;
						break;
					}
				}
			}
		}
	}
protected:
	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto port = g_ObjectMgr->GetDirectory(WKD_Device).Open("portC", access).MoveAs<PortDevice>();
		d0Pin_ = port->OpenPin(PortPins::Pin8);
		d1Pin_ = port->OpenPin(PortPins::Pin9);
		d2Pin_ = port->OpenPin(PortPins::Pin10);
		d3Pin_ = port->OpenPin(PortPins::Pin11);
		ckPin_ = port->OpenPin(PortPins::Pin12);

		port = g_ObjectMgr->GetDirectory(WKD_Device).Open("portD", access).MoveAs<PortDevice>();
		cmdPin_ = port->OpenPin(PortPins::Pin2);

		d0Pin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		d1Pin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		d2Pin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		d3Pin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		ckPin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);
		cmdPin_->SetMode(PortOutputMode::AF_PushPull, PortOutputSpeed::PS_50MHz);

		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::SDIO, true);
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(RccPeriph::SDIO, false);
		d0Pin_.Reset();
		d1Pin_.Reset();
		d2Pin_.Reset();
		d3Pin_.Reset();
		ckPin_.Reset();
		cmdPin_.Reset();
	}
private:
	void SetBlockSize(sdio_dctrl& dctrl, size_t blockSize)
	{
		auto dblk = std::log2((float)blockSize);

		if (dblk < 0 || dblk > 14 || dblk != (size_t)dblk)
			throw std::invalid_argument("Invalid block size.");

		dctrl.DBLOCKSIZE = static_cast<sdio_dblksize>(size_t(dblk));
	}
private:
	const FDTDevice& fdt_;
	SDIO_TypeDef* sdio_;
	ObjectAccessor<PortPin> d0Pin_, d1Pin_, d2Pin_, d3Pin_, ckPin_, cmdPin_;
};

SdioDriver::SdioDriver(const FDTDevice& device)
	:device_(device)
{

}

void SdioDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<Stm32SdioController>(device_));
}
