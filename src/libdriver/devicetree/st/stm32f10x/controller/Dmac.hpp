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

		enum class DmaRequestLine
		{
			I2C1_TX,
			I2C1_RX
		};

		struct DmaTransferOptions
		{
			DmaTransmition Type;
			uintptr_t SourceAddress;
			size_t SourceBitwidth;
			bool SourceInc;
			uintptr_t DestAddress;
			size_t DestBitwidth;
			bool DestInc;
			size_t Count;
		};

		class DmaChannel : public Object
		{
		public:
			virtual void Configure(const DmaTransferOptions& options) = 0;
			virtual ObjectPtr<IAsyncAction> StartAsync() = 0;
		};

		class DmaController : public Device
		{
		public:
			virtual ObjectPtr<DmaChannel> OpenChannel(DmaRequestLine requestLine) = 0;
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
