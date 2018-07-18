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
			double AccelerationX;
			double AccelerationY;
			double AccelerationZ;
		};

		class Accelerometer : public Device
		{
		public:
			virtual AccelerometerReading GetCurrentReading() = 0;
		};
	}
}
