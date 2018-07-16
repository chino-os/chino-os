//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

namespace Chino
{
	namespace Device
	{
		struct AccelerometerReading
		{
			float AccelerationX;
			float AccelerationY;
			float AccelerationZ;
		};

		class Accelerometer : public Device
		{
		public:
			virtual AccelerometerReading GetCurrentReading() = 0;
		};
	}
}
