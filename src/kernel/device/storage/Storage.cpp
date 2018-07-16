//
// Kernel Device
//
#include "Storage.hpp"
#include "../../kdebug.hpp"

using namespace Chino;
using namespace Chino::Device;

DeviceType StorageDevice::GetType() const noexcept
{
	return DeviceType::Storage;
}

StorageType EEPROMStorage::GetStorageType()
{
	return StorageType::EEPROM;
}
