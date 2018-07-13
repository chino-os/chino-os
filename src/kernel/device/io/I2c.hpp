//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

#include <cstdint>
#include <cstddef>
#include <gsl/gsl>

namespace Chino
{
	namespace Device
	{
		class I2cDevice : public Device
		{
		public:
			virtual size_t Read(gsl::span<uint8_t> buffer) = 0;
			virtual void Write(gsl::span<const uint8_t> buffer) = 0;
		};

		class I2cController : public Device
		{
		public:
			virtual ObjectAccessor<I2cDevice> OpenDevice(uint32_t slaveAddress, ObjectAccess access) = 0;
		};
	}
}
