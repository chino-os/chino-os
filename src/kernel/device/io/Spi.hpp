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
			Mode0,		//!< CPOL = 0, CPHA = 0
			Mode1,		//!< CPOL = 0, CPHA = 1
			Mode2,		//!< CPOL = 1, CPHA = 0
			Mode3		//!< CPOL = 1, CPHA = 1
		};

		class SpiDevice : public Device
		{
		public:
			virtual void Write(BufferList<const uint8_t> bufferList) = 0;
			virtual void TransferFullDuplex(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) = 0;
			virtual void TransferSequential(BufferList<const uint8_t> writeBufferList, BufferList<uint8_t> readBufferList) = 0;
		};

		struct ChipSelectPin
		{
			virtual void Activate() = 0;
			virtual void Deactivate() = 0;
		};

		class SpiController : public Device
		{
		public:
			virtual ObjectAccessor<SpiDevice> OpenDevice(uint32_t chipSelectMask, SpiMode mode, uint32_t dataBitLength, ObjectAccess access) = 0;
			virtual ObjectAccessor<SpiDevice> OpenDevice(ChipSelectPin& csPin, SpiMode mode, uint32_t dataBitLength, ObjectAccess access) = 0;
		};
	}
}
