//
// Kernel Device
//
#include "enc28j60.hpp"
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

DEFINE_FDT_DRIVER_DESC_1(ENC28J60Driver, "ethernet-controller", "microchip,enc28j60");

enum bank_t
{
	NOBANK = 0,
	EIE = 0x1B,
	EIR = 0x1C,
	ESTAT = 0x1D,
	ECON2 = 0x1E,
	ECON1 = 0x1F
};

// Bank 0 registers
#define ERDPTL           (0x00|0x00)
#define ERDPTH           (0x01|0x00)
#define EWRPTL           (0x02|0x00)
#define EWRPTH           (0x03|0x00)
#define ETXSTL           (0x04|0x00)
#define ETXSTH           (0x05|0x00)
#define ETXNDL           (0x06|0x00)
#define ETXNDH           (0x07|0x00)
#define ERXSTL           (0x08|0x00)
#define ERXSTH           (0x09|0x00)
#define ERXNDL           (0x0A|0x00)
#define ERXNDH           (0x0B|0x00)

#define ERXRDPTL         (0x0C|0x00)
#define ERXRDPTH         (0x0D|0x00)
#define ERXWRPTL         (0x0E|0x00)
#define ERXWRPTH         (0x0F|0x00)
#define EDMASTL          (0x10|0x00)
#define EDMASTH          (0x11|0x00)
#define EDMANDL          (0x12|0x00)
#define EDMANDH          (0x13|0x00)
#define EDMADSTL         (0x14|0x00)
#define EDMADSTH         (0x15|0x00)
#define EDMACSL          (0x16|0x00)
#define EDMACSH          (0x17|0x00)
// Bank 1 registers
#define EHT0             (0x00|0x20)
#define EHT1             (0x01|0x20)
#define EHT2             (0x02|0x20)
#define EHT3             (0x03|0x20)
#define EHT4             (0x04|0x20)
#define EHT5             (0x05|0x20)
#define EHT6             (0x06|0x20)
#define EHT7             (0x07|0x20)
#define EPMM0            (0x08|0x20)
#define EPMM1            (0x09|0x20)
#define EPMM2            (0x0A|0x20)
#define EPMM3            (0x0B|0x20)
#define EPMM4            (0x0C|0x20)
#define EPMM5            (0x0D|0x20)
#define EPMM6            (0x0E|0x20)
#define EPMM7            (0x0F|0x20)
#define EPMCSL           (0x10|0x20)
#define EPMCSH           (0x11|0x20)
#define EPMOL            (0x14|0x20)
#define EPMOH            (0x15|0x20)
#define EWOLIE           (0x16|0x20)
#define EWOLIR           (0x17|0x20)
#define ERXFCON          (0x18|0x20)
#define EPKTCNT          (0x19|0x20)
// Bank 2 registers
#define MACON1           (0x00|0x40|0x80)
#define MACON2           (0x01|0x40|0x80)
#define MACON3           (0x02|0x40|0x80)
#define MACON4           (0x03|0x40|0x80)
#define MABBIPG          (0x04|0x40|0x80)
#define MAIPGL           (0x06|0x40|0x80)
#define MAIPGH           (0x07|0x40|0x80)
#define MACLCON1         (0x08|0x40|0x80)
#define MACLCON2         (0x09|0x40|0x80)
#define MAMXFLL          (0x0A|0x40|0x80)
#define MAMXFLH          (0x0B|0x40|0x80)
#define MAPHSUP          (0x0D|0x40|0x80)
#define MICON            (0x11|0x40|0x80)
#define MICMD            (0x12|0x40|0x80)
#define MIREGADR         (0x14|0x40|0x80)
#define MIWRL            (0x16|0x40|0x80)
#define MIWRH            (0x17|0x40|0x80)
#define MIRDL            (0x18|0x40|0x80)
#define MIRDH            (0x19|0x40|0x80)
// Bank 3 registers
#define MAADR1           (0x00|0x60|0x80)
#define MAADR0           (0x01|0x60|0x80)
#define MAADR3           (0x02|0x60|0x80)
#define MAADR2           (0x03|0x60|0x80)
#define MAADR5           (0x04|0x60|0x80)
#define MAADR4           (0x05|0x60|0x80)
#define EBSTSD           (0x06|0x60)
#define EBSTCON          (0x07|0x60)
#define EBSTCSL          (0x08|0x60)
#define EBSTCSH          (0x09|0x60)
#define MISTAT           (0x0A|0x60|0x80)
#define EREVID           (0x12|0x60)
#define ECOCON           (0x15|0x60)
#define EFLOCON          (0x17|0x60)
#define EPAUSL           (0x18|0x60)
#define EPAUSH           (0x19|0x60)
// PHY registers
#define PHCON1           0x00
#define PHSTAT1          0x01
#define PHHID1           0x02
#define PHHID2           0x03
#define PHCON2           0x10
#define PHSTAT2          0x11
#define PHIE             0x12
#define PHIR             0x13
#define PHLCON           0x14	   
// ENC28J60 ERXFCON Register Bit Definitions
#define ERXFCON_UCEN     0x80
#define ERXFCON_ANDOR    0x40
#define ERXFCON_CRCEN    0x20
#define ERXFCON_PMEN     0x10
#define ERXFCON_MPEN     0x08
#define ERXFCON_HTEN     0x04
#define ERXFCON_MCEN     0x02
#define ERXFCON_BCEN     0x01
// ENC28J60 EIE Register Bit Definitions
#define EIE_INTIE        0x80
#define EIE_PKTIE        0x40
#define EIE_DMAIE        0x20
#define EIE_LINKIE       0x10
#define EIE_TXIE         0x08
#define EIE_WOLIE        0x04
#define EIE_TXERIE       0x02
#define EIE_RXERIE       0x01
// ENC28J60 EIR Register Bit Definitions
#define EIR_PKTIF        0x40
#define EIR_DMAIF        0x20
#define EIR_LINKIF       0x10
#define EIR_TXIF         0x08
#define EIR_WOLIF        0x04
#define EIR_TXERIF       0x02
#define EIR_RXERIF       0x01
// ENC28J60 ESTAT Register Bit Definitions
#define ESTAT_INT        0x80
#define ESTAT_LATECOL    0x10
#define ESTAT_RXBUSY     0x04
#define ESTAT_TXABRT     0x02
#define ESTAT_CLKRDY     0x01
// ENC28J60 ECON2 Register Bit Definitions
#define ECON2_AUTOINC    0x80
#define ECON2_PKTDEC     0x40
#define ECON2_PWRSV      0x20
#define ECON2_VRPS       0x08
// ENC28J60 ECON1 Register Bit Definitions
#define ECON1_TXRST      0x80
#define ECON1_RXRST      0x40
#define ECON1_DMAST      0x20
#define ECON1_CSUMEN     0x10
#define ECON1_TXRTS      0x08
#define ECON1_RXEN       0x04
#define ECON1_BSEL1      0x02
#define ECON1_BSEL0      0x01
// ENC28J60 MACON1 Register Bit Definitions
#define MACON1_LOOPBK    0x10
#define MACON1_TXPAUS    0x08
#define MACON1_RXPAUS    0x04
#define MACON1_PASSALL   0x02
#define MACON1_MARXEN    0x01
// ENC28J60 MACON2 Register Bit Definitions
#define MACON2_MARST     0x80
#define MACON2_RNDRST    0x40
#define MACON2_MARXRST   0x08
#define MACON2_RFUNRST   0x04
#define MACON2_MATXRST   0x02
#define MACON2_TFUNRST   0x01
// ENC28J60 MACON3 Register Bit Definitions
#define MACON3_PADCFG2   0x80
#define MACON3_PADCFG1   0x40
#define MACON3_PADCFG0   0x20
#define MACON3_TXCRCEN   0x10
#define MACON3_PHDRLEN   0x08
#define MACON3_HFRMLEN   0x04
#define MACON3_FRMLNEN   0x02
#define MACON3_FULDPX    0x01
// ENC28J60 MICMD Register Bit Definitions
#define MICMD_MIISCAN    0x02
#define MICMD_MIIRD      0x01
// ENC28J60 MISTAT Register Bit Definitions
#define MISTAT_NVALID    0x04
#define MISTAT_SCAN      0x02
#define MISTAT_BUSY      0x01
// ENC28J60 PHY PHCON1 Register Bit Definitions
#define PHCON1_PRST      0x8000
#define PHCON1_PLOOPBK   0x4000
#define PHCON1_PPWRSV    0x0800
#define PHCON1_PDPXMD    0x0100
// ENC28J60 PHY PHSTAT1 Register Bit Definitions
#define PHSTAT1_PFDPX    0x1000
#define PHSTAT1_PHDPX    0x0800
#define PHSTAT1_LLSTAT   0x0004
#define PHSTAT1_JBSTAT   0x0002
// ENC28J60 PHY PHCON2 Register Bit Definitions
#define PHCON2_FRCLINK   0x4000
#define PHCON2_TXDIS     0x2000
#define PHCON2_JABBER    0x0400
#define PHCON2_HDLDIS    0x0100

enum operation_t
{
	READ_CTRL_REG = 0x00,
	READ_BUF_MEM = 0x3A,
	WRITE_CTRL_REG = 0x40,
	WRITE_BUF_MEM = 0x7A,
	BIT_FIELD_SET = 0x80,
	BIT_FIELD_CLR = 0xA0,
	SOFT_RESET = 0xFF
};

#define ADDR_MASK		0x1F
#define BANK_MASK		0x60
#define SPRD_MASK		0x80

class ENC28J60Device : public EthernetController, public ExclusiveObjectAccess
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
	ENC28J60Device(const FDTDevice& fdt)
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
		g_Logger->PutFormat("Reset\n");
		WriteOp(SOFT_RESET, 0, SOFT_RESET);
		while (!(Read(ESTAT) & ESTAT_CLKRDY));

		auto id = Read(EREVID);
		g_Logger->PutFormat("ID: %x\n", id);

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
		Write(MIREGADR, addr);
		Write(MIWRL, data);
		Write(MIWRH, data >> 8);
		while (Read(MISTAT) & MISTAT_BUSY);
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

ENC28J60Driver::ENC28J60Driver(const FDTDevice& device)
	:device_(device)
{

}

void ENC28J60Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<ENC28J60Device>(device_));
}
