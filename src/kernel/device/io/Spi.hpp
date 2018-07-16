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
		class SpiDevice : public Device
		{
		public:
			virtual size_t WriteRead(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) = 0;
			virtual void Write(BufferList<const uint8_t> bufferList) = 0;
		};

		class SpiController : public Device
		{
		public:
			virtual ObjectAccessor<SpiDevice> OpenDevice(uint32_t chipSelectLine, ObjectAccess access) = 0;
		};
	}
}
