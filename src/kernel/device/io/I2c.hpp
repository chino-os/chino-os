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
		class I2cDevice : public Device
		{
		public:
			virtual size_t WriteRead(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) = 0;
			virtual void Write(BufferList<const uint8_t> bufferList) = 0;
		};

		class I2cController : public Device
		{
		public:
			virtual ObjectAccessor<I2cDevice> OpenDevice(uint32_t slaveAddress, ObjectAccess access) = 0;
		};
	}
}
