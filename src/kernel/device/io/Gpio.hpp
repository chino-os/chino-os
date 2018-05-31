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
		enum class GpioPinValue
		{
			Low,
			High
		};

		class GpioPin : public Device
		{
		public:
			virtual GpioPinValue Read() const = 0;
			virtual void Write(GpioPinValue value) = 0;
		};

		class GpioController : public Device
		{
		public:
			virtual size_t GetPinCount() const = 0;
			virtual ObjectPtr<GpioPin> GetPin(size_t index) = 0;
		};
	}
}
