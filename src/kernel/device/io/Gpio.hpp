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

		enum class GpioPinDriveMode
		{
			Input,
			InputPullDown,
			InputPullUp,
			Output
		};

		class GpioPin : public Device
		{
		public:
			virtual GpioPinValue Read() = 0;
			virtual void Write(GpioPinValue value) = 0;

			virtual void SetDriveMode(GpioPinDriveMode driveMode) = 0;
		};

		class GpioController : public Device
		{
		public:
			virtual size_t GetPinCount() const = 0;
			virtual ObjectAccessor<GpioPin> OpenPin(size_t index, ObjectAccess access) = 0;
		};
	}
}
