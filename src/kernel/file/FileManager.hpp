//
// Kernel File
//
#pragma once
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <string_view>
#include "../utils.hpp"
#include "../device/storage/filesystem/FileSystem.hpp"
#include "../../libchino/chino.h"

namespace Chino
{
	namespace File
	{
		class FileManager
		{
			template<class T>
			struct PathMap
			{
				std::unordered_map<std::string, std::reference_wrapper<T>> PathMap;
				size_t NextAvailId;

				void Add(std::string prefix, T& device)
				{
					PathMap.emplace(prefix + std::to_string(NextAvailId), device);
					NextAvailId++;
				}
			};

			struct File
			{
				enum
				{
					FILE_FREE,
					FILE_FS
				} Tag;

				File()
					:Tag(FILE_FREE)
				{

				}

				File(std::unique_ptr<Device::FileSystemFile>&& file)
					:FSFile(std::move(file)), Tag(FILE_FS)
				{
				}

				std::unique_ptr<Device::FileSystemFile> FSFile;
			};
		public:
			HANDLE OpenFile(std::string_view fileName);
			size_t GetFileSize(HANDLE file);
			void ReadFile(HANDLE file, uint8_t* buffer, size_t offset, size_t length);

			void RegisterFileSystems(Device::FileSystem& fileSystem);

			void DumpFileSystems();
			File& FindEmptyFileSlot(HANDLE& handle);
		private:
			Device::FileSystemFile& GetFSFile(HANDLE handle);
		private:
			std::vector<std::reference_wrapper<Device::FileSystem>> fileSystems_;
			PathMap<Device::FileSystem> fsPathMap_;
			std::array<File, 256> files_;
		};
	}
}

extern StaticHolder<Chino::File::FileManager> g_FileMgr;