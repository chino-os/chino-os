//
// Kernel Device
//
#pragma once
#include "../volume/Partition.hpp"
#include <functional>

namespace Chino
{
	namespace Device
	{
		class Iso9600FileSystem : public Driver
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
			};
#pragma pack(pop)
		public:
			DECLARE_PARTITION_DRIVER(Iso9600FileSystem);

			Iso9600FileSystem(Partition& partition);

			virtual void Install() override;
		private:
			void ForEachPathTable(std::function<bool(const PathEntry&)> callback);
		private:
			Partition & partition_;

			uint32_t pathTableLBA_;
			uint32_t pathTableSize_;
		};
	}
}
