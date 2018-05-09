//
// Kernel Device
//
#pragma once
#include "../volume/Partition.hpp"

namespace Chino
{
	namespace Device
	{
		class Iso9600FileSystem : public Driver
		{
		public:
			DECLARE_PARTITION_DRIVER(Iso9600FileSystem);

			Iso9600FileSystem(Partition& partition);

			virtual void Install() override;
		private:
			Partition & partition_;

			uint8_t buffer[2048];
		};
	}
}
