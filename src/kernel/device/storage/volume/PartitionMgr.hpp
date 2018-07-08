//
// Kernel Device
//
#pragma once

#include "../Drive.hpp"
#include "Partition.hpp"
#include <vector>

namespace Chino
{
	namespace Device
	{
		class ParitionManager : public Driver
		{
		public:
			DECLARE_DRIVE_DRIVER(ParitionManager);

			ParitionManager(DriveDevice& device);

			virtual void Install() override;
		private:
			void InstallPartition(ObjectPtr<Partition> partition);
		private:
			ObjectPtr<DriveDevice> drive_;
			std::vector<ObjectPtr<Partition>> partitions_;
		};
	}
}
