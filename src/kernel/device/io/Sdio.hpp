//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

namespace Chino
{
	namespace Device
	{
		enum class SdioCommandIndex : uint32_t
		{
			GO_IDLE_STATE = 0,
			ALL_SEND_CID = 2,
			SEND_RELATIVE_ADDR = 3,
			SEL_DESEL_CARD = 7,
			SEND_IF_COND = 8,
			SEND_CSD = 9,
			READ_SINGLE_BLOCK = 17,
			APP_CMD = 55,
			ACMD_SD_SET_BUSWIDTH = 6,
			ACMD_SD_SEND_OP_COND = 41,
		};

		enum class SdioResponseType
		{
			None,
			Short,
			Long
		};

		enum class SdioResponseFormat
		{
			None,
			R1,
			R1b,
			R2,
			R3,
			R4,
			R4b,
			R5,
			R6,
			R7
		};

		enum class SdioDatabusWidth
		{
			One,
			Four
		};

		struct SdioCommand
		{
			SdioCommandIndex CommandIndex;
			SdioResponseType ResponseType;
			SdioResponseFormat ResponseFormat;
			uint32_t Argument;
		};

		struct SdioResponse
		{
			std::array<uint32_t, 4> Data;
		};

		class SdioController : public Device
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver() override;

			virtual void Reset() = 0;
			virtual void SetDatabusWidth(SdioDatabusWidth width) = 0;
			virtual void SendCommand(const SdioCommand& command) = 0;
			virtual void SendCommand(const SdioCommand& command, SdioResponse& response) = 0;

			virtual void ReadDataBlocks(const SdioCommand& command, size_t blockSize, size_t blocksCount, BufferList<uint8_t> bufferList) = 0;
		};
	}
}
