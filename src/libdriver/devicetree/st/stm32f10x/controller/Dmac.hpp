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
			I2C1_RX,
			SPI2_TX,
			SPI2_RX
		};

		struct DmaSessionHandler
		{
			virtual void OnStart() = 0;
			virtual void OnStop() = 0;
		};

		class DmaChannel : public Object
		{
		public:
			virtual ObjectPtr<IAsyncAction> StartAsync() = 0;

			template<class TSource, class TDest>
			void Configure(DmaTransmition type, BufferList<const TSource> source, BufferList<TDest> dest, size_t count = 0, DmaSessionHandler* handler = nullptr)
			{
				auto srcCast = source.Select().template Cast<const volatile uint8_t>();
				auto destCast = dest.Select().template Cast<volatile uint8_t>();
				ConfigureImpl(type, srcCast.AsBufferList(), destCast.AsBufferList(), sizeof(TSource), sizeof(TDest), count, handler);
			}
		protected:
			virtual void ConfigureImpl(DmaTransmition type, BufferList<const volatile uint8_t> source, BufferList<volatile uint8_t> dest, size_t sourceByteWidth, size_t destByteWidth, size_t count, DmaSessionHandler* handler) = 0;
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
