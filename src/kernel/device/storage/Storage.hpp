//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>

namespace Chino
{
	namespace Device
	{
		enum class StorageType
		{
			Disk,
			Flash,
			EEPROM,
			Other
		};

		class StorageDevice : public Device
		{
		public:
			virtual DeviceType GetType() const noexcept override;

			virtual StorageType GetStorageType() = 0;
			virtual size_t GetSize() = 0;
			virtual size_t Read(size_t offset, BufferList<uint8_t> bufferList) = 0;
			virtual void Write(size_t offset, BufferList<const uint8_t> bufferList) = 0;
		};

		class EEPROMStorage : public StorageDevice
		{
		public:
			virtual StorageType GetStorageType() override final;
		};

		class FlashStorage : public StorageDevice
		{
		public:
			virtual StorageType GetStorageType() override final;

			virtual size_t GetSectorSize() = 0;
			virtual size_t GetSectorsCount() = 0;
			virtual void EraseSector(size_t sectorId) = 0;
			virtual void EraseAllSectors() = 0;
		};
	}
}
