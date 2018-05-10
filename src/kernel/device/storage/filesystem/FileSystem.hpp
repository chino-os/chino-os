//
// Kernel Device
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string_view>

namespace Chino
{
	namespace Device
	{
		class FileSystem;

		struct FileSystemFile
		{
			FileSystem& FS;

			size_t NumBlocks;
			size_t DataLength;
		};

		using FilePath = std::string_view;

		class FileSystem
		{
		public:
			virtual std::unique_ptr<FileSystemFile> TryOpenFile(const FilePath& filePath) = 0;
			virtual void ReadFile(FileSystemFile& file, uint8_t* buffer, size_t blockOffset, size_t numBlocks) = 0;

			size_t BlockSize;
			std::string Name;
		};
	}
}
