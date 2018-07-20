//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
	namespace Device
	{
		struct FsmcSuppress
		{
			FsmcSuppress();
			~FsmcSuppress();
		};

		struct NorSramTiming
		{
			uint32_t AddressSetupTime;
			uint32_t AddressHoldTime;
			uint32_t DataSetupTime;
		};

		enum class FsmcBank1Sectors
		{
			Sram1,
			Sram2,
			Sram3,
			Sram4
		};

		class FsmcController : public Device
		{
		public:
			virtual void ConfigureSram(FsmcBank1Sectors sector, const NorSramTiming& readingTiming, const NorSramTiming& writingTiming) = 0;
		};

		class FsmcDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(FsmcDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
