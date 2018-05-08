//
// Kernel Device
//
#include "PartitionMgr.hpp"
#include <portable.h>
#include "../../../memory/MemoryManager.hpp"
#include <cstring>

using namespace Chino::Device;

DEFINE_DRIVE_DRIVER_DESC(ParitionManager);

ParitionManager::ParitionManager(const DriveDevice& device)
{
}

bool ParitionManager::IsSupported(const Chino::Device::DriveDevice& device)
{
	return true;
}

void ParitionManager::Install()
{
}
