//
// Kernel Device
//
#pragma once
#include "../Driver.hpp"

#include <cstdint>
#include <cstddef>

namespace Chino
{
	namespace Device
	{
		class SerialIODevice
		{
		public:
			virtual size_t Read(uint8_t* buffer, size_t count) = 0;
			virtual size_t Write(const uint8_t* buffer, size_t count) = 0;
		};
	}
}
