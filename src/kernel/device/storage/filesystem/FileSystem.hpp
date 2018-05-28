//
// Kernel Device
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include "path.hpp"
#include "../Drive.hpp"

namespace Chino
{
	namespace Device
	{
		class FileSystem;

		struct FileSystemFile : public Object
		{
			FileSystem& FS;

			size_t DataLength;

			FileSystemFile(FileSystem& fs)
				:FS(fs) {}
		};

		class FileSystem : public Driver
		{
		public:
			virtual ObjectPtr<FileSystemFile> TryOpenFile(const Path& filePath) = 0;
			virtual void ReadFile(FileSystemFile& file, uint8_t* buffer, size_t blockOffset, size_t numBlocks) = 0;

			size_t BlockSize;
			std::string Name;
		};
	}
}
