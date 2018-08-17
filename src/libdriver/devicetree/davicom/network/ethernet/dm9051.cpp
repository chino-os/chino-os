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

DEFINE_FDT_DRIVER_DESC_1(DM9051Driver, "ethernet-controller", "microchip,enc28j60");
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
		return Read(EPKTCNT);
	}

	virtual void Reset(const MacAddress& macAddress) override
	{
		WriteOp(SOFT_RESET, 0, SOFT_RESET);
		while (!(Read(ESTAT) & ESTAT_CLKRDY));

		packetPtr_ = 0;
		SetPacketPtr(0);
		SetReceiveBuffer(ReceiveBufferFirst, ReceiveBufferLast);
		SetSendBuffer(SendBufferFirst, SendBufferLast);
		SetReceiveFilter(ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
		Write(EPMM0, 0x3f);
		Write(EPMM1, 0x30);
		Write(EPMCSL, 0xf9);
		Write(EPMCSH, 0xf7);

		Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
		Write(MACON2, 0x00);
		WriteOp(BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);

		Write(MAIPGL, 0x12);
		Write(MAIPGH, 0x0C);
		Write(MABBIPG, 0x15);

		SetMaxFrameLength(MaxFrameSize);
		SetMacAddress(macAddress);

		WritePhy(PHCON1, PHCON1_PDPXMD);
		WritePhy(PHCON2, PHCON2_HDLDIS);

		SetBank(ECON1);
		WriteOp(BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
		WriteOp(BIT_FIELD_SET, ECON1, ECON1_RXEN);
		kassert(Read(MAADR5) == macAddress[0]);
		WritePhy(PHLCON, 0x0476);;
	}

	virtual void StartSend(size_t length) override
	{
		while ((Read(ECON1) & ECON1_TXRTS) != 0);

		Write(EWRPTL, SendBufferFirst & 0xFF);
		Write(EWRPTH, SendBufferFirst >> 8);

		/* 设置发送缓冲区结束地址 该值对应发送数据包长度 */
		Write(ETXNDL, (SendBufferFirst + length) & 0xFF);
		Write(ETXNDH, (SendBufferFirst + length) >> 8);

		/* 发送控制字节 控制字节为0x00,表示使用macon3设置 */
		WriteOp(WRITE_BUF_MEM, 0, 0x00);
	}

	virtual void WriteSendBuffer(gsl::span<const uint8_t> buffer) override
	{
		const uint8_t cmd[] = { WRITE_BUF_MEM };
		gsl::span<const uint8_t> writeBuffers[] = { cmd, buffer };
		dev_->Write({ writeBuffers });
	}

	virtual void CommitSend() override
	{
		WriteOp(BIT_FIELD_SET, ECON1, ECON1_TXRTS);

		if ((Read(EIR) & EIR_TXERIF))
		{
			SetBank(ECON1);
			WriteOp(BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
		}
	}

	virtual size_t StartReceive() override
	{
		/* 数据包总长度 */
		int len = 0;
		int rxstat;

		Write(ERDPTL, packetPtr_);
		Write(ERDPTH, packetPtr_ >> 8);

		/* 接收数据包结构示例 数据手册43页 */
		/* 读下一个包的指针 */
		packetPtr_ = ReadOp(READ_BUF_MEM, 0);
		packetPtr_ |= ReadOp(READ_BUF_MEM, 0) << 8;

		/* 读包的长度 */
		len = ReadOp(READ_BUF_MEM, 0);
		len |= ReadOp(READ_BUF_MEM, 0) << 8;

		/* 删除CRC计数 */
		len -= 4;

		/* 读取接收状态 */
		rxstat = ReadOp(READ_BUF_MEM, 0);
		rxstat |= ReadOp(READ_BUF_MEM, 0) << 8;

		/* 注意取消了CRC校验检查部分 */
		/* 返回接收数据包长度 */
		return len;
	}

	virtual void ReadReceiveBuffer(gsl::span<uint8_t> buffer) override
	{
		const uint8_t cmd[] = { READ_BUF_MEM };
		gsl::span<const uint8_t> writeBuffers[] = { cmd };
		gsl::span<uint8_t> readBuffers[] = { buffer };
		dev_->TransferSequential({ writeBuffers }, { readBuffers });
	}

	virtual void FinishReceive() override
	{
		Write(ERXRDPTL, (packetPtr_));
		Write(ERXRDPTH, (packetPtr_) >> 8);

		WriteOp(BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
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

		lastBank_ = NOBANK;
	}

	virtual void OnLastClose() override
	{
		dev_.Reset();
		csPin_.Reset();
	}
private:
	void SetPacketPtr(uint16_t ptr)
	{
		Write(ERXRDPTL, uint8_t(ptr));
		Write(ERXRDPTL, uint8_t(ptr >> 8));
	}

	void SetReceiveBuffer(uint16_t first, uint16_t last)
	{
		Write(ERXSTL, uint8_t(first));
		Write(ERXSTH, uint8_t(first >> 8));
		Write(ERXNDL, uint8_t(last));
		Write(ERXNDH, uint8_t(last >> 8));
	}

	void SetSendBuffer(uint16_t first, uint16_t last)
	{
		Write(ETXSTL, uint8_t(first));
		Write(ETXSTH, uint8_t(first >> 8));
		Write(ETXNDL, uint8_t(last));
		Write(ETXNDH, uint8_t(last >> 8));
	}

	void SetReceiveFilter(uint8_t filter)
	{
		Write(ERXFCON, filter);
	}

	void SetMaxFrameLength(uint16_t len)
	{
		Write(MAMXFLL, uint8_t(len));
		Write(MAMXFLH, uint8_t(len >> 8));
	}

	void SetMacAddress(const MacAddress& macAddress)
	{
		Write(MAADR5, macAddress[0]);
		Write(MAADR4, macAddress[1]);
		Write(MAADR3, macAddress[2]);
		Write(MAADR2, macAddress[3]);
		Write(MAADR1, macAddress[4]);
		Write(MAADR0, macAddress[5]);
	}

	void SetBank(bank_t bank)
	{
		if (bank != lastBank_)
		{
			WriteOp(BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
			WriteOp(BIT_FIELD_SET, ECON1, bank >> 5);
			lastBank_ = bank;
		}
	}

	void WritePhy(uint8_t addr, uint16_t data)
	{
		/* Fill the phyxcer register into REG_0C                                                                */
		DM9051_Write_Reg(DM9051_EPAR, DM9051_PHY | reg);

		/* Fill the written data into REG_0D & REG_0E */
		DM9051_Write_Reg(DM9051_EPDRL, (value & 0xff));
		DM9051_Write_Reg(DM9051_EPDRH, ((value >> 8) & 0xff));
		/* Issue phyxcer write command */
		DM9051_Write_Reg(DM9051_EPCR, 0xa);

		/* Wait write complete */
		//_DM9051_Delay_ms(500);                       
		while (DM9051_Read_Reg(DM9051_EPCR) & 0x1) { _DM9051_Delay(1); }; //Wait complete

																		  /* Clear phyxcer write command */
		DM9051_Write_Reg(DM9051_EPCR, 0x0);
	}

	void SetPhyMode(uint32_t mode)
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
			phy_write(4, phy_reg4);
			/* Write rphy_reg0 to Tmp */
			phy_write(0, phy_reg0);
			_DM9051_Delay(10);
		}
	}

	void WriteOp(operation_t op, uint8_t addr, uint8_t data)
	{
		const uint8_t toWrite[] = { static_cast<uint8_t>(op | (addr & ADDR_MASK)), data };
		gsl::span<const uint8_t> buffers[] = { toWrite };
		dev_->Write({ buffers });
	}

	uint8_t ReadOp(operation_t op, uint8_t addr)
	{
		const uint8_t toWrite[1] = { static_cast<uint8_t>(op | (addr & ADDR_MASK)) };
		gsl::span<const uint8_t> writeBuffers[] = { toWrite };

		// if MAC/MII the 2nd byte is valid
		if (addr & 0x80)
		{
			uint8_t toRead[2];
			gsl::span<uint8_t> readBuffers[] = { toRead };

			dev_->TransferSequential({ writeBuffers }, { readBuffers });
			return toRead[1];
		}
		else
		{
			uint8_t toRead[1];
			gsl::span<uint8_t> readBuffers[] = { toRead };

			dev_->TransferSequential({ writeBuffers }, { readBuffers });
			return toRead[0];
		}
	}

	uint8_t Read(uint8_t addr)
	{
		SetBank(static_cast<bank_t>(addr & BANK_MASK));
		return ReadOp(READ_CTRL_REG, addr);
	}

	void Write(uint8_t addr, uint8_t data)
	{
		SetBank(static_cast<bank_t>(addr & BANK_MASK));
		WriteOp(WRITE_CTRL_REG, addr, data);
	}
private:
	const FDTDevice& fdt_;
	ObjectAccessor<SpiDevice> dev_;
	ObjectAccessor<GpioPin> csPin_;
	CS cs_;
	bank_t lastBank_;
	uint16_t packetPtr_;
};

DM9051Driver::DM9051Driver(const FDTDevice& device)
	:device_(device)
{

}

void DM9051Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<DM9051Device>(device_));
}
