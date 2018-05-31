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
			virtual void Start() = 0;
			virtual void Stop() = 0;

			virtual size_t Read(ObjectAccessContext& context, uint8_t* buffer, size_t count) = 0;
			virtual size_t Write(ObjectAccessContext& context, const uint8_t* buffer, size_t count) = 0;
		};
	}
}
