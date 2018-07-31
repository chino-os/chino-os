//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>
#include "Rcc.hpp"
#include <kernel/device/Async.hpp>

namespace Chino
{
	namespace Device
	{
		enum class DmaTransmition
		{
			Perip2Mem,
			Mem2Periph,
			Mem2Mem
		};

		class DmaChannel : public Object
		{
		public:
			virtual void Configure(DmaTransmition transmition, uintptr_t sourceAddr, uintptr_t destAddr, size_t sourceWidth, size_t destWidth, size_t count) = 0;
			virtual ObjectPtr<IAsyncAction> StartAsync() = 0;
		};

		class DmaController : public Device
		{
		public:
			virtual ObjectPtr<DmaChannel> OpenChannel(RccPeriph periph) = 0;
		};

		class DmacDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(DmacDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};
	}
}
