//
// Kernel Device
//
#include "Partition.hpp"
#include "../../../kdebug.hpp"

using namespace Chino::Device;

Partition::Partition(DriveDevice& drive, size_t startLBA)
	:drive_(drive), startLBA_(startLBA), MaxLBA(drive.MaxLBA), BlockSize(drive.BlockSize)
{

}

std::unique_ptr<Driver> Partition::TryLoadDriver()
{
	auto head = g_PartitionDrivers;
	auto cnt = *head;

	while (cnt)
	{
		if (cnt->IsSupported(*this))
			return cnt->Activator(*this);

		cnt = *++head;
	}

	return nullptr;
}

void Partition::Read(uint64_t lba, size_t count, uint8_t* buffer)
{
	lba += startLBA_;
	drive_.Read(lba, count, buffer);
}
