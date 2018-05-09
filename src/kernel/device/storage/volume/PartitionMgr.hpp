//
// Kernel Device
//
#pragma once

#include "../Drive.hpp"
#include "Partition.hpp"
#include <list>

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
			void InstallPartition(Partition&& partition);
		private:
			DriveDevice & drive_;
			std::list<Partition> partitions_;
		};
	}
}
