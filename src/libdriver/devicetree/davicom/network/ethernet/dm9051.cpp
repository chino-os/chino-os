//
// Kernel Device
//
#include "dm9051.hpp"
#include <kernel/device/io/Spi.hpp>
#include <kernel/device/io/Gpio.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <libbsp/bsp.hpp>
#include <kernel/device/network/Ethernet.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(DM9051Driver, "ethernet-controller", "davicom,dm9051");
/* Private typedef -----------------------------------------------------------------------------------------*/
enum DM9051_PHY_mode
{
	DM9051_10MHD = 0,
	DM9051_100MHD = 1,
	DM9051_10MFD = 4,
	DM9051_100MFD = 5,
	DM9051_10M = 6,
	DM9051_AUTO = 8,
	DM9051_1M_HPNA = 0x10
};

enum DM9051_TYPE
{
	TYPE_DM9051E,
	TYPE_DM9051A,
	TYPE_DM9051B,
	TYPE_DM9051
};

struct DM9051_eth
{
	enum DM9051_TYPE type;
	enum DM9051_PHY_mode mode;

	uint8_t  imr_all;

	/* packet I or II */
	uint8_t  packet_cnt;
	/* queued packet (packet II) */
	uint16_t queue_packet_len;

	/* interface address info. */
	uint8_t  dev_addr[6]; /* hw address */

						  /* Byte counter */
						  //uint32_t_t txb_count;	
						  //uint32_t_t rxb_count;  
};
static struct DM9051_eth DM9051_device;

/* Private constants ---------------------------------------------------------------------------------------*/
#define DM9051_PHY          (0x40)    			/* PHY address 0x01                                             */

/* Exported typedef ----------------------------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------------------------------------*/
#define DM9051_ID           (0x90510A46)		/* DM9051A ID                                                   */
#define DM9051_PKT_MAX      (1536)          /* Received packet max size                                     */
#define DM9051_PKT_RDY      (0x01)          /* Packet ready to receive                                      */

#define DM9051_NCR          (0x00)
#define DM9051_NSR          (0x01)
#define DM9051_TCR          (0x02)
#define DM9051_TSR1         (0x03)
#define DM9051_TSR2         (0x04)
#define DM9051_RCR          (0x05)
#define DM9051_RSR          (0x06)
#define DM9051_ROCR         (0x07)
#define DM9051_BPTR         (0x08)
#define DM9051_FCTR         (0x09)
#define DM9051_FCR          (0x0A)
#define DM9051_EPCR         (0x0B)
#define DM9051_EPAR         (0x0C)
#define DM9051_EPDRL        (0x0D)
#define DM9051_EPDRH        (0x0E)
#define DM9051_WCR          (0x0F)

#define DM9051_PAR          (0x10)
#define DM9051_MAR          (0x16)

#define DM9051_GPCR         (0x1e)
#define DM9051_GPR          (0x1f)
#define DM9051_TRPAL        (0x22)
#define DM9051_TRPAH        (0x23)
#define DM9051_RWPAL        (0x24)
#define DM9051_RWPAH        (0x25)

#define DM9051_VIDL         (0x28)
#define DM9051_VIDH         (0x29)
#define DM9051_PIDL         (0x2A)
#define DM9051_PIDH         (0x2B)

#define DM9051_CHIPR        (0x2C)
#define DM9051_TCR2         (0x2D)
#define DM9051_OTCR         (0x2E)
#define DM9051_SMCR         (0x2F)

#define DM9051_ETCR         (0x30)    /* early transmit control/status register                             */
#define DM9051_CSCR         (0x31)    /* check sum control register                                         */
#define DM9051_RCSSR        (0x32)    /* receive check sum status register                                  */

#define DM9051_PBCR					(0x38)
#define DM9051_INTR					(0x39)
#define DM9051_MPCR					(0x55)
#define DM9051_MRCMDX       (0x70)
#define DM9051_MRCMDX1      (0x71)
#define DM9051_MRCMD        (0x72)
#define DM9051_MRRL         (0x74)
#define DM9051_MRRH         (0x75)
#define DM9051_MWCMDX       (0x76)
#define DM9051_MWCMD        (0x78)
#define DM9051_MWRL         (0x7A)
#define DM9051_MWRH         (0x7B)
#define DM9051_TXPLL        (0x7C)
#define DM9051_TXPLH        (0x7D)
#define DM9051_ISR          (0x7E)
#define DM9051_IMR          (0x7F)

#define CHIPR_DM9051A       (0x19)
#define CHIPR_DM9051B       (0x1B)

#define DM9051_REG_RESET    (0x01)
#define DM9051_IMR_OFF      (0x80)
#define DM9051_TCR2_SET     (0x90)	/* set one packet */
#define DM9051_RCR_SET      (0x31)
#define DM9051_BPTR_SET     (0x37)
#define DM9051_FCTR_SET     (0x38)
#define DM9051_FCR_SET      (0x28)
#define DM9051_TCR_SET      (0x01)


#define NCR_EXT_PHY         (1 << 7)
#define NCR_WAKEEN          (1 << 6)
#define NCR_FCOL            (1 << 4)
#define NCR_FDX             (1 << 3)
#define NCR_LBK             (3 << 1)
#define NCR_RST             (1 << 0)
#define NCR_DEFAULT					(0x0)    /* Disable Wakeup */

#define NSR_SPEED           (1 << 7)
#define NSR_LINKST          (1 << 6)
#define NSR_WAKEST          (1 << 5)
#define NSR_TX2END          (1 << 3)
#define NSR_TX1END          (1 << 2)
#define NSR_RXOV            (1 << 1)
#define NSR_CLR_STATUS			(NSR_WAKEST | NSR_TX2END | NSR_TX1END)

#define TCR_TJDIS           (1 << 6)
#define TCR_EXCECM          (1 << 5)
#define TCR_PAD_DIS2        (1 << 4)
#define TCR_CRC_DIS2        (1 << 3)
#define TCR_PAD_DIS1        (1 << 2)
#define TCR_CRC_DIS1        (1 << 1)
#define TCR_TXREQ           (1 << 0) /* Start TX */
#define TCR_DEFAULT					(0x0)

#define TSR_TJTO            (1 << 7)
#define TSR_LC              (1 << 6)
#define TSR_NC              (1 << 5)
#define TSR_LCOL            (1 << 4)
#define TSR_COL             (1 << 3)
#define TSR_EC              (1 << 2)

#define RCR_WTDIS           (1 << 6)
#define RCR_DIS_LONG        (1 << 5)
#define RCR_DIS_CRC         (1 << 4)
#define RCR_ALL             (1 << 3)
#define RCR_RUNT            (1 << 2)
#define RCR_PRMSC           (1 << 1)
#define RCR_RXEN            (1 << 0)
#define RCR_DEFAULT					(RCR_DIS_LONG | RCR_DIS_CRC)

#define RSR_RF              (1 << 7)
#define RSR_MF              (1 << 6)
#define RSR_LCS             (1 << 5)
#define RSR_RWTO            (1 << 4)
#define RSR_PLE             (1 << 3)
#define RSR_AE              (1 << 2)
#define RSR_CE              (1 << 1)
#define RSR_FOE             (1 << 0)

#define BPTR_DEFAULT				(0x3f)
#define FCTR_DEAFULT				(0x38)
#define FCR_DEFAULT					(0xFF)
#define SMCR_DEFAULT				(0x0)
#define PBCR_MAXDRIVE				(0x44)

//#define FCTR_HWOT(ot)       ((ot & 0xF ) << 4 )
//#define FCTR_LWOT(ot)       (ot & 0xF )

#define IMR_PAR             (1 << 7)
#define IMR_LNKCHGI         (1 << 5)
#define IMR_UDRUN						(1 << 4)
#define IMR_ROOM            (1 << 3)
#define IMR_ROM             (1 << 2)
#define IMR_PTM             (1 << 1)
#define IMR_PRM             (1 << 0)
#define IMR_FULL 						(IMR_PAR | IMR_LNKCHGI | IMR_UDRUN | IMR_ROOM | IMR_ROM | IMR_PTM | IMR_PRM)
#define IMR_OFF							(IMR_PAR)
#define IMR_DEFAULT					(IMR_PAR | IMR_PRM | IMR_PTM) 

#define ISR_ROOS            (1 << 3)
#define ISR_ROS             (1 << 2)
#define ISR_PTS             (1 << 1)
#define ISR_PRS             (1 << 0)
#define ISR_CLR_STATUS      (0x80 | 0x3F)

#define EPCR_REEP           (1 << 5)
#define EPCR_WEP            (1 << 4)
#define EPCR_EPOS           (1 << 3)
#define EPCR_ERPRR          (1 << 2)
#define EPCR_ERPRW          (1 << 1)
#define EPCR_ERRE           (1 << 0)

#define GPCR_GEP_CNTL       (1 << 0)

#define SPI_WR_BURST				(0xF8)
#define SPI_RD_BURST				(0x72)

#define SPI_READ						(0x03)
#define SPI_WRITE        		(0x04)
#define SPI_WRITE_BUFFER  	(0x05)		/* Send a series of bytes from the Master to the Slave */
#define SPI_READ_BUFFER   	(0x06)    /* Send a series of bytes from the Slave  to the Master */

class DM9051Device : public EthernetController, public ExclusiveObjectAccess
{
	struct CS : public ChipSelectPin
	{
		CS(ObjectAccessor<GpioPin>& pin)
			:pin_(pin)
		{
		}

		virtual void Activate() override
		{
			pin_->Write(GpioPinValue::Low);
		}

		virtual void Deactivate() override
		{
			pin_->Write(GpioPinValue::High);
		}
	private:
		ObjectAccessor<GpioPin>& pin_;
	};

	enum
	{
		BufferSize = 8 * 1024,	// 8KB,
		MaxFrameSize = 1518,	// Max MAC frame
		SendBufferLast = BufferSize - 1,
		SendBufferSize = MaxFrameSize,
		SendBufferFirst = SendBufferLast - SendBufferSize,
		ReceiveBufferFirst = 0,
		ReceiveBufferLast = SendBufferFirst - 1,
		ReceiveBufferSize = ReceiveBufferLast - ReceiveBufferFirst + 1
	};
public:
	DM9051Device(const FDTDevice& fdt)
		:fdt_(fdt), cs_(csPin_)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void SetHandler(INetworkHandler* handler) override
	{

	}

	virtual bool IsPacketAvailable() override
	{
		//g_Logger->PutFormat("H: %x\n", Read(DM9051_MRCMDX));
		return (Read(DM9051_MRCMDX) & 0b11) == 0b1;
	}

	virtual void Reset(const MacAddress& macAddress) override
	{
		Write(DM9051_NCR, DM9051_REG_RESET);
		while (Read(DM9051_NCR) & DM9051_REG_RESET);

		Write(DM9051_GPCR, GPCR_GEP_CNTL);
		Write(DM9051_GPR, 0x00);		//Power on PHY
		BSPSleepMs(100);

		SetPhyMode(DM9051_10MFD);
		SetMacAddress(macAddress);

		/* set multicast address */
		for (size_t i = 0; i < 8; i++)
		{ /* Clear Multicast set */
								  /* Set Broadcast */
			Write(DM9051_MAR + i, (7 == i) ? 0x80 : 0x00);
		}

		/************************************************
		*** Activate DM9051 and Setup DM9051 Registers **
		*************************************************/
		/* Clear DM9051 Set and Disable Wakeup function */
		Write(DM9051_NCR, NCR_DEFAULT);
		/* Clear TCR Register set */
		Write(DM9051_TCR, TCR_DEFAULT);
		/* Discard long Packet and CRC error Packet */
		Write(DM9051_RCR, RCR_DEFAULT);
		/*  Set 1.15 ms Jam Pattern Timer */
		Write(DM9051_BPTR, BPTR_DEFAULT);

		/* Open / Close Flow Control */
		//DM9051_Write_Reg(DM9051_FCTR, FCTR_DEAFULT);
		Write(DM9051_FCTR, 0x3A);
		Write(DM9051_FCR, FCR_DEFAULT);

		/* Set Memory Conttrol Register，TX = 3K，RX = 13K */
		Write(DM9051_SMCR, SMCR_DEFAULT);
		/* Set Send one or two command Packet*/
		Write(DM9051_TCR2, DM9051_TCR2_SET);

		//DM9051_Write_Reg(DM9051_TCR2, 0x80);
		Write(DM9051_INTR, 0x1);

		/* Clear status */
		Write(DM9051_NSR, NSR_CLR_STATUS);
		Write(DM9051_ISR, ISR_CLR_STATUS);

		kassert(Read(DM9051_PAR) == macAddress[0]);

		Write(DM9051_IMR, IMR_PAR | IMR_PRM);
		Write(DM9051_RCR, (RCR_DEFAULT | RCR_RXEN));  /* Enable RX */
	}

	virtual void StartSend(size_t length) override
	{
		kassert(length <= std::numeric_limits<uint16_t>::max());
		while (Read(DM9051_TCR) & DM9051_TCR_SET);

		calcMWR_ = (Read(DM9051_MWRH) << 8) + Read(DM9051_MWRL);
		calcMWR_ += length;
		if (calcMWR_ > 0x0bff) calcMWR_ -= 0x0c00;

		Write(DM9051_TXPLL, length & 0xff);
		Write(DM9051_TXPLH, (length >> 8) & 0xff);

		//if (calcMWR_ != ((Read(DM9051_MWRH) << 8) + Read(DM9051_MWRL)))
		//{
		//	g_Logger->PutString("WTF\n");
		//	/*若是指針出錯，將指針移到下一個傳送包的包頭位置  */
		//	Write(DM9051_MWRH, (calcMWR_ >> 8) & 0xff);
		//	Write(DM9051_MWRL, calcMWR_ & 0xff);
		//}
	}

	virtual void WriteSendBuffer(gsl::span<const uint8_t> buffer) override
	{
		gsl::span<const uint8_t> buffers[] = { buffer };
		WriteMemory({ buffers });
	}

	virtual void CommitSend() override
	{
		/* Issue TX polling command */
		Write(DM9051_TCR, TCR_TXREQ); /* Cleared after TX complete */
	}

	virtual size_t StartReceive() override
	{
		/* Check packet ready or not */
		uint8_t rxbyte = Read(DM9051_MRCMDX);
		rxbyte = Read(DM9051_MRCMDX);

		if ((rxbyte != 1) && (rxbyte != 0))
		{
			/* Reset RX FIFO pointer */
			Write(DM9051_RCR, RCR_DEFAULT);	//RX disable
			Write(DM9051_MPCR, 0x01);		//Reset RX FIFO pointer
			BSPSleepMs(20);
			Write(DM9051_RCR, (RCR_DEFAULT | RCR_RXEN));		//RX Enable
			return 0;
		}

		Read(DM9051_MRCMDX);		// dummy read

		calcMRR_ = (Read(DM9051_MRRH) << 8) + Read(DM9051_MRRL);	// Save RX SRAM pointer
		uint16_t status, len;
		{
			uint8_t header[4];
			gsl::span<uint8_t> readBuffers[] = { header };
			ReadMemory({ readBuffers });
			status = header[0] | (header[1] << 8);
			len = header[2] | (header[3] << 8);
		}

		calcMRR_ += (len + 4);
		if (calcMRR_ > 0x3fff) calcMRR_ -= 0x3400;
		return len;
	}

	virtual void ReadReceiveBuffer(gsl::span<uint8_t> buffer) override
	{
		gsl::span<uint8_t> readBuffers[] = { buffer };
		ReadMemory({ readBuffers });
	}

	virtual void FinishReceive() override
	{
		return;
		//if (calcMRR_ != ((Read(DM9051_MRRH) << 8) + Read(DM9051_MRRL)))
		//{
		//	printf("DM9K MRR Error!!\r\n");
		//	printf("Predicut RX Read pointer = 0x%X, Current pointer = 0x%X\r\n", calcMRR_, ((Read(DM9051_MRRH) << 8) + Read(DM9051_MRRL)));
		//
		//	/*若是指針出錯，將指針移到下一個包的包頭位置  */
		//	Write(DM9051_MRRH, (calcMRR_ >> 8) & 0xff);
		//	Write(DM9051_MRRL, calcMRR_ & 0xff);
		//}
	}
protected:
	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;
		auto spi = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("spi")->GetString(), access).MoveAs<SpiController>();
		dev_ = spi->OpenDevice(cs_, SpiMode::Mode0, 8, access);
		auto gpio = g_ObjectMgr->GetDirectory(WKD_Device).Open(fdt_.GetProperty("cs_gpio")->GetString(), access).MoveAs<GpioController>();
		csPin_ = gpio->OpenPin(fdt_.GetProperty("cs_pin")->GetUInt32(0), access);
		csPin_->SetDriveMode(GpioPinDriveMode::Output);
		csPin_->Write(GpioPinValue::High);

		//uint32_t value = 0;
		///* Read DM9051 PID / VID, Check MCU SPI Setting correct */
		//value |= (uint32_t)Read(DM9051_VIDL);
		//value |= (uint32_t)Read(DM9051_VIDH) << 8;
		//value |= (uint32_t)Read(DM9051_PIDL) << 16;
		//value |= (uint32_t)Read(DM9051_PIDH) << 24;

		//g_Logger->PutFormat("ID: %x\n", value);
	}

	virtual void OnLastClose() override
	{
		dev_.Reset();
		csPin_.Reset();
	}
private:
	void SetMacAddress(const MacAddress& macAddress)
	{
		Write(DM9051_PAR + 0, macAddress[0]);
		Write(DM9051_PAR + 1, macAddress[1]);
		Write(DM9051_PAR + 2, macAddress[2]);
		Write(DM9051_PAR + 3, macAddress[3]);
		Write(DM9051_PAR + 4, macAddress[4]);
		Write(DM9051_PAR + 5, macAddress[5]);
	}

	void SetPhyMode(uint32_t uMediaMode)
	{
		uint16_t phy_reg4 = 0x01e1, phy_reg0 = 0x1000;

		if (!(uMediaMode & DM9051_AUTO))
		{
			switch (uMediaMode)
			{
			case DM9051_10MHD:
			{
				phy_reg4 = 0x21;
				phy_reg0 = 0x0000;
				break;
			}
			case DM9051_10MFD:
			{
				phy_reg4 = 0x41;
				phy_reg0 = 0x1100;
				break;
			}
			case DM9051_100MHD:
			{
				phy_reg4 = 0x81;
				phy_reg0 = 0x2000;
				break;
			}
			case DM9051_100MFD:
			{
				phy_reg4 = 0x101;
				phy_reg0 = 0x3100;
				break;
			}
			case DM9051_10M:
			{
				phy_reg4 = 0x61;
				phy_reg0 = 0x1200;
				break;
			}
			}

			/* Set PHY media mode */
			WritePhy(4, phy_reg4);
			/* Write rphy_reg0 to Tmp */
			WritePhy(0, phy_reg0);
			BSPSleepMs(10);
		}
	}

	void WritePhy(uint8_t addr, uint16_t data)
	{
		/* Fill the phyxcer register into REG_0C */
		Write(DM9051_EPAR, DM9051_PHY | addr);

		/* Fill the written data into REG_0D & REG_0E */
		Write(DM9051_EPDRL, (data & 0xff));
		Write(DM9051_EPDRH, ((data >> 8) & 0xff));
		/* Issue phyxcer write command */
		Write(DM9051_EPCR, 0xa);

		/* Wait write complete */
		//_DM9051_Delay_ms(500);                       
		while (Read(DM9051_EPCR) & 0x1) { BSPSleepMs(1); }; //Wait complete

		/* Clear phyxcer write command */
		Write(DM9051_EPCR, 0x0);
	}

	uint16_t ReadPhy(uint8_t addr)
	{
		/* Fill the phyxcer register into REG_0C */
		Write(DM9051_EPAR, DM9051_PHY | addr);
		/* Issue phyxcer read command */
		Write(DM9051_EPCR, 0xc);

		/* Wait read complete */
		//_DM9051_Delay_ms(100);                        
		while (Read(DM9051_EPCR) & 0x1) { BSPSleepMs(1); }; //Wait complete

		/* Clear phyxcer read command */
		Write(DM9051_EPCR, 0x0);
		return (Read(DM9051_EPDRH) << 8) | Read(DM9051_EPDRL);
	}

	void ReadMemory(BufferList<uint8_t> bufferList)
	{
		const uint8_t toWrite[1] = { SPI_RD_BURST };
		gsl::span<const uint8_t> writeBuffers[] = { toWrite };

		dev_->TransferSequential({ writeBuffers }, bufferList);
	}

	void WriteMemory(BufferList<const uint8_t> bufferList)
	{
		const uint8_t toWrite[1] = { SPI_WR_BURST };
		auto buffers = bufferList.Select().Prepend({ toWrite });
		dev_->Write(buffers.AsBufferList());
	}

	uint8_t Read(uint8_t addr)
	{
		const uint8_t toWrite[1] = { addr };
		gsl::span<const uint8_t> writeBuffers[] = { toWrite };

		uint8_t toRead[1];
		gsl::span<uint8_t> readBuffers[] = { toRead };

		dev_->TransferSequential({ writeBuffers }, { readBuffers });
		return toRead[0];
	}

	void Write(uint8_t addr, uint8_t data)
	{
		const uint8_t toWrite[] = { static_cast<uint8_t>(addr | 0x80), data };
		gsl::span<const uint8_t> buffers[] = { toWrite };
		dev_->Write({ buffers });
	}
private:
	const FDTDevice& fdt_;
	ObjectAccessor<SpiDevice> dev_;
	ObjectAccessor<GpioPin> csPin_;
	CS cs_;
	uint16_t calcMWR_, calcMRR_;
};

DM9051Driver::DM9051Driver(const FDTDevice& device)
	:device_(device)
{

}

void DM9051Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<DM9051Device>(device_));
}
