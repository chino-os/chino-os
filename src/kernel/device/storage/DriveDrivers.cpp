//
// Kernel Device
//
#include "Drive.hpp"
#include "filesystem/PartitionMgr.hpp"

using namespace Chino::Device;

#define REF_DRIVE_DRIVER_DESC(Type) &Type::Descriptor

const DriveDriverDescriptor* Chino::Device::g_DriveDrivers[] =
{
	REF_DRIVE_DRIVER_DESC(ParitionManager),
	nullptr
};
