//
// Kernel Device
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include "path.hpp"

namespace Chino
{
	namespace Device
	{
		class FileSystem;

		struct FileSystemFile
		{
			FileSystem& FS;

			size_t DataLength;

			FileSystemFile(FileSystem& fs)
				:FS(fs) {}
		};

		class FileSystem
		{
		public:
			virtual std::unique_ptr<FileSystemFile> TryOpenFile(const Path& filePath) = 0;
			virtual void ReadFile(FileSystemFile& file, uint8_t* buffer, size_t blockOffset, size_t numBlocks) = 0;

			size_t BlockSize;
			std::string Name;
		};
	}
}
