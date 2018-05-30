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
		class Serial : public Device
		{
		public:
			virtual void Open() = 0;
			virtual void Close() = 0;

			virtual size_t Read(uint8_t* buffer, size_t count) = 0;
			virtual size_t Write(const uint8_t* buffer, size_t count) = 0;
		};
	}
}
