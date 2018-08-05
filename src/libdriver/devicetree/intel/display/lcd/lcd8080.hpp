//
// Kernel Device
//
#pragma once
#include <kernel/device/Device.hpp>

namespace Chino
{
	namespace Device
	{
		class LCD8080Controller : public Device
		{
		public:
			virtual void WriteRead(uint16_t reg, BufferList<const uint16_t> writeBufferList, BufferList<uint16_t> readBufferList) = 0;
			virtual void Write()
			virtual void Fill(uint16_t reg, uint16_t value, size_t count) = 0;
		};
	}
}
