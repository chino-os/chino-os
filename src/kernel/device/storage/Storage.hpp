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
			SD,
			Other
		};

		class StorageDevice : public Device
		{
		public:
			virtual DeviceType GetType() const noexcept override;

			virtual StorageType GetStorageType() = 0;
			virtual uint64_t GetSize() = 0;
		};

		class EEPROMStorage : public StorageDevice
		{
		public:
			virtual StorageType GetStorageType() override final;

			virtual size_t Read(size_t offset, BufferList<uint8_t> bufferList) = 0;
			virtual void Write(size_t offset, BufferList<const uint8_t> bufferList) = 0;
		};

		class FlashStorage : public StorageDevice
		{
		public:
			virtual StorageType GetStorageType() override final;

			virtual size_t GetSectorSize() = 0;
			virtual size_t GetSectorsCount() = 0;
			//virtual size_t GetPageSize() = 0;
			//virtual void ProgramPage(BufferList<const uint8_t> bufferList) = 0;
			virtual void EraseSector(size_t sectorId) = 0;
			virtual void EraseAllSectors() = 0;
		};

		class SDStorage : public StorageDevice
		{
		public:
			virtual StorageType GetStorageType() override final;

			virtual size_t GetReadWriteBlockSize() = 0;
			virtual size_t GetEraseSectorSize() = 0;

			virtual void ReadBlocks(size_t startBlock, size_t blocksCount, BufferList<uint8_t> bufferList) = 0;
			virtual void WriteBlocks(size_t startBlock, size_t blocksCount, BufferList<const uint8_t> bufferList) = 0;
		};
	}
}
