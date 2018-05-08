//
// Kernel Device
//
#pragma once

#include "../Drive.hpp"

namespace Chino
{
	namespace Device
	{
		class ParitionManager : public Driver
		{
		public:
			DECLARE_DRIVE_DRIVER(ParitionManager);

			ParitionManager(const DriveDevice& device);

			virtual void Install() override;
		};
	}
}
