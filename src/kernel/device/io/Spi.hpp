//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

#include <cstdint>
#include <cstddef>

namespace Chino
{
	namespace Device
	{
		enum class SpiMode
		{
			Mode0,
			Mode1,
			Mode2,
			Mode3
		};

		class SpiDevice : public Device
		{
		public:
			virtual void Write(BufferList<const uint8_t> bufferList) = 0;
			virtual void TransferFullDuplex(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) = 0;
			virtual void TransferSequential(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) = 0;
		};

		class SpiController : public Device
		{
		public:
			virtual ObjectAccessor<SpiDevice> OpenDevice(uint32_t chipSelectLine, uint32_t dataBitLength, ObjectAccess access) = 0;
		};
	}
}
