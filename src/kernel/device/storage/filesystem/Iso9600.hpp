//
// Kernel Device
//
#pragma once
#include "../volume/Partition.hpp"
#include "FileSystem.hpp"
#include <functional>

namespace Chino
{
	namespace Device
	{
		class Iso9600FileSystem : public Driver, public FileSystem
		{
#pragma pack(push, 1)
			struct PathEntry
			{
				uint8_t IdentifierLength;
				uint8_t ExtendedAttributeLength;
				uint32_t ExtentLBA;
				uint16_t ParentNumber;
				uint8_t Identifier[256];
			};

			static_assert(sizeof(PathEntry) == 8 + 256, "Bad PathEntry layout.");

			struct DirectoryEntry
			{
				uint8_t Length;
				uint8_t ExtendedAttributeLength;
				uint32_t ExtentLBA;
				uint32_t ExtentLBA_BE;
				uint32_t DataLength;
				uint32_t DataLength_BE;
				uint8_t DateTime[7];
				uint8_t FileFlags;
				uint8_t FileUnitSize;
				uint8_t GapSize;
				uint16_t VolumeSequenceNumber;
				uint16_t VolumeSequenceNumber_BE;
				uint8_t IdentifierLength;
				uint8_t IdentifierAndSystemUse[256 - 33];
			};

			static_assert(sizeof(DirectoryEntry) == 256, "Bad DirectoryEntry layout.");
#pragma pack(pop)
		public:
			DECLARE_PARTITION_DRIVER(Iso9600FileSystem);

			Iso9600FileSystem(Partition& partition);

			virtual void Install() override;

			virtual std::unique_ptr<FileSystemFile> TryOpenFile(const Path& filePath);
			virtual void ReadFile(FileSystemFile& file, uint8_t* buffer, size_t blockOffset, size_t numBlocks);
		private:
			void ForEachPathTable(std::function<bool(const PathEntry&)> callback);
			void ForEachDirectoryEntry(uint32_t lba, std::function<bool(const DirectoryEntry&)> callback);
		private:
			Partition & partition_;

			uint32_t pathTableLBA_;
			uint32_t pathTableSize_;
		};
	}
}
