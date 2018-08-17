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
			SEND_IF_COND = 8,
			SEND_CSD = 9,
			APP_CMD = 55,
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
			virtual void SendCommand(const SdioCommand& command) = 0;
			virtual void SendCommand(const SdioCommand& command, SdioResponse& response) = 0;
		};
	}
}
