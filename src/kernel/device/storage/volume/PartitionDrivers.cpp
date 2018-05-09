//
// Kernel Device
//
#include "Partition.hpp"
#include "../filesystem/Iso9600.hpp"

using namespace Chino::Device;

#define REF_PARITION_DRIVER_DESC(Type) &Type::Descriptor

const PartitionDriverDescriptor* Chino::Device::g_PartitionDrivers[] =
{
	REF_PARITION_DRIVER_DESC(Iso9600FileSystem),
	nullptr
};
