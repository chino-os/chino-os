//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

#include <cstdint>
#include <cstddef>
#include <gsl/span>

namespace Chino
{
	namespace Device
	{
		enum class Parity
		{
			None,
			Odd,
			Even,
			Mark,
			Space
		};

		enum class StopBits
		{
			None,
			One,
			OnePointFive,
			Two
		};

		class Serial : public Device
		{
		public:
			virtual void SetBaudRate(size_t baudRate) = 0;
			virtual void SetParity(Parity parity) = 0;
			virtual void SetStopBits(StopBits stopBits) = 0;
			virtual void SetDataBits(size_t dataBits) = 0;

			virtual size_t Read(gsl::span<uint8_t> buffer) = 0;
			virtual void Write(gsl::span<const uint8_t> buffer) = 0;
		};
	}
}
