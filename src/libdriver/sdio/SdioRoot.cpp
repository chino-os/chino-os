//
// Kernel Device
//
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/io/Sdio.hpp>

using namespace Chino;
using namespace Chino::Device;

#define SD_VOLTAGE_WINDOW_SD            0x80100000
#define SD_HIGH_CAPACITY                0x40000000
#define SD_STD_CAPACITY                 0x00000000
#define SD_CHECK_PATTERN                0x000001AA

#define SD_MAX_VOLT_TRIAL               0x0000FFFF

#define SD_R6_GENERAL_UNKNOWN_ERROR     0x00002000
#define SD_R6_ILLEGAL_CMD               0x00004000
#define SD_R6_COM_CRC_FAILED            0x00008000

struct SDCard
{
	std::array<uint32_t, 4> CID;
	uint16_t RCA;

	uint32_t SectorSize;
	uint64_t Capacity;
};

#pragma pack(push, 1)
union sdio_csd
{
	struct
	{
		uint32_t RESV0 : 1;
		uint32_t CRC : 7;

		uint32_t RESV1 : 2;
		uint32_t FILE_FORMAT : 2;
		uint32_t TMP_WRITE_PROTECT : 1;
		uint32_t PERM_WRITE_PROTECT : 1;
		uint32_t COPY : 1;
		uint32_t FILE_FORMAT_GRP : 1;

		uint32_t RESV3 : 5;
		uint32_t WRITE_BL_PARTIAL : 1;
		uint32_t WRITE_BL_LEN : 4;
		uint32_t R2W_FACTOR : 3;

		uint32_t RESV4 : 2;
		uint32_t WP_GRP_ENABLE : 1;
		uint32_t WP_GRP_SIZE : 7;
		uint32_t SECTOR_SIZE : 7;
		uint32_t ERASE_BLK_EN : 1;

		uint32_t RESV5 : 1;
		uint32_t C_SIZE : 22;

		uint32_t RESV6 : 6;
		uint32_t DSR_IMP : 1;
		uint32_t READ_BLK_MISALIGN : 1;
		uint32_t WRITE_BLK_MISALIGN : 1;
		uint32_t READ_BL_PARTIAL : 1;
		uint32_t READ_BL_LEN : 4;

		uint32_t CCC : 12;
		uint32_t TRAN_SPEED : 8;
		uint32_t NSAC : 8;
		uint32_t TAAC : 8;

		uint32_t RESV7 : 6;
		uint32_t CSD_STRUCTURE : 2;
	};

	std::array<uint32_t, 4> Value;
};

static_assert(sizeof(sdio_csd) == 16, "This pack may work on GCC only.");
#pragma pack(pop)

class SdioRootDriver : public Driver
{
	struct IdentContext
	{
		uint32_t SDType;
		bool PowerUp;

		std::array<uint32_t, 4> CID;
		uint16_t RCA;
		std::array<uint32_t, 4> CSD;
	};
public:
	SdioRootDriver(ObjectPtr<SdioController> sdio)
		:sdio_(MakeAccessor(std::move(sdio), OA_Read | OA_Write))
	{
	}

	virtual void Install() override
	{
		EnumerateDevices();
	}
private:
	void EnumerateDevices()
	{
		sdio_->Reset();
		SendGoIdle();

		IdentContext contex{};
		CheckVersion(contex);
		GetCID(contex);
		GetRCA(contex);
		GetCSD(contex);
		AddSDCard(contex);
	}
private:
	void SendGoIdle()
	{
		SdioCommand cmd{ SdioCommandIndex::GO_IDLE_STATE, SdioResponseType::None, SdioResponseFormat::None };
		sdio_->SendCommand(cmd);
	}

	void AddSDCard(IdentContext& contex)
	{
		SDCard card;
		card.CID = contex.CID;
		card.RCA = contex.RCA;

		sdio_csd csd;
		csd.Value[0] = contex.CSD[3];
		csd.Value[1] = contex.CSD[2];
		csd.Value[2] = contex.CSD[1];
		csd.Value[3] = contex.CSD[0];

		kassert(csd.CSD_STRUCTURE == 0b01);
		card.SectorSize = csd.SECTOR_SIZE;
		card.Capacity = 512ull * 1024 * (csd.C_SIZE + 1);
		g_Logger->PutFormat("Size: %z Mbytes\n", size_t(card.Capacity / 1024 / 1024));
	}

	void CheckVersion(IdentContext& contex)
	{
		contex.SDType = SD_STD_CAPACITY;

		SdioCommand cmd{ SdioCommandIndex::SEND_IF_COND, SdioResponseType::Short, SdioResponseFormat::R7, SD_CHECK_PATTERN };
		SdioResponse resp;

		bool isV2 = false;
		try
		{
			sdio_->SendCommand(cmd, resp);
			isV2 = true;
			contex.SDType = SD_HIGH_CAPACITY;
		}
		catch (std::runtime_error&)
		{
			g_Logger->PutFormat("SD V1.0\n");
			throw std::runtime_error("Not supported.");
		}

		if (isV2)
		{
			if (resp.Data[0] != SD_CHECK_PATTERN)
				throw std::runtime_error("Invalid SD card.");
			g_Logger->PutFormat("SD V2.0: %x\n", resp.Data[0]);
		}

		SendAppCmd();
		SetVoltage(contex);
	}

	void GetCID(IdentContext& contex)
	{
		SdioCommand cmd{ SdioCommandIndex::ALL_SEND_CID, SdioResponseType::Long, SdioResponseFormat::R2 };
		SdioResponse resp;
		sdio_->SendCommand(cmd, resp);
		contex.CID = resp.Data;
	}

	void GetRCA(IdentContext& contex)
	{
		SdioCommand cmd{ SdioCommandIndex::SEND_RELATIVE_ADDR, SdioResponseType::Short, SdioResponseFormat::R6 };
		SdioResponse resp;
		sdio_->SendCommand(cmd, resp);

		if (resp.Data[0] & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED))
			throw std::runtime_error("Failed to get RCA.");
		contex.RCA = uint16_t(resp.Data[0] >> 16);
		g_Logger->PutFormat("RCA: %x\n", contex.RCA);
	}

	void GetCSD(IdentContext& contex)
	{
		SdioCommand cmd{ SdioCommandIndex::SEND_CSD, SdioResponseType::Long, SdioResponseFormat::R2, uint32_t(contex.RCA << 16) };
		SdioResponse resp;
		sdio_->SendCommand(cmd, resp);
		contex.CSD = resp.Data;
	}

	void SetVoltage(IdentContext& contex)
	{
		for (size_t i = 0; i < SD_MAX_VOLT_TRIAL; i++)
		{
			SendAppCmd();
			SdioCommand cmd{ SdioCommandIndex::ACMD_SD_SEND_OP_COND, SdioResponseType::Short, SdioResponseFormat::R3, SD_VOLTAGE_WINDOW_SD | contex.SDType };
			SdioResponse resp;
			sdio_->SendCommand(cmd, resp);

			if (resp.Data[0] >> 31)
			{
				contex.PowerUp = true;
				break;
			}
		}

		if (!contex.PowerUp)
			throw std::runtime_error("Invalid voltage.");
	}

	void SendAppCmd()
	{
		SdioCommand cmd{ SdioCommandIndex::APP_CMD, SdioResponseType::Short, SdioResponseFormat::R1, 0 };
		SdioResponse resp;
		sdio_->SendCommand(cmd, resp);
	}
private:
	ObjectAccessor<SdioController> sdio_;
};

Chino::ObjectPtr<Driver> Chino::Device::BSPInstallSdioRootDriver(ObjectPtr<SdioController> sdio)
{
	return MakeObject<SdioRootDriver>(std::move(sdio));
}
