//
// Kernel Device
//
#include "PartitionMgr.hpp"
#include "../../DeviceManager.hpp"
#include <portable.h>
#include <cstring>

using namespace Chino::Device;

DEFINE_DRIVE_DRIVER_DESC(ParitionManager);

struct PartitionTableDetector
{

};

ParitionManager::ParitionManager(DriveDevice& device)
	:drive_(device)
{
}

bool ParitionManager::IsSupported(Chino::Device::DriveDevice& device)
{
	return true;
}

void ParitionManager::Install()
{
	// only support 1 partition now
	InstallPartition({ drive_, 0 });
}

void ParitionManager::InstallPartition(Partition&& partition)
{
	auto& ref = partitions_.emplace_back(std::move(partition));

	auto driver = ref.TryLoadDriver();
	if (driver)
		g_DeviceMgr->InstallDriver(std::move(driver));
}
