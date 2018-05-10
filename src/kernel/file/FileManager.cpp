//
// Kernel File
//
#include "FileManager.hpp"
#include "../kdebug.hpp"

using namespace Chino::File;

void FileManager::RegisterFileSystems(Device::FileSystem& fileSystem)
{
	fileSystems_.emplace_back(fileSystem);
	fsPathMap_.Add("fs", fileSystem);
}

void FileManager::DumpFileSystems()
{
	g_BootVideo->PutString("====== Dump File Systems ======\n");
	for (auto& dev : fsPathMap_.PathMap)
	{
		g_BootVideo->PutFormat("/dev/%s: Format: %s, Block Size: %d\n", dev.first.c_str(), dev.second.get().Name.c_str(), (int)dev.second.get().BlockSize);
	}
}

HANDLE FileManager::OpenFile(std::string_view fileName)
{
	kassert(fileName.compare(0, 7, "/dev/fs") == 0);
	std::string dev(fileName.substr(4, fileName.find_first_of('/', 5) - 4));
	auto fs = fsPathMap_.PathMap.find(dev);
	if (fs == fsPathMap_.PathMap.end()) return 0;
	auto file = fs->second.get().TryOpenFile(fileName.substr(fileName.find_first_of('/', 5)));
	if (file)
	{
		HANDLE handle;
		auto& slot = FindEmptyFileSlot(handle);
		slot = { std::move(file) };
		return handle;
	}

	return 0;
}

FileManager::File& FileManager::FindEmptyFileSlot(HANDLE& handle)
{
	for (size_t i = 0; i < files_.size(); i++)
	{
		if (files_[i].Tag == File::FILE_FREE)
		{
			handle = i + 1;
			return files_[i];
		}
	}

	kassert(!"No free file handle slot.");
}

Chino::Device::FileSystemFile& FileManager::GetFSFile(HANDLE handle)
{
	kassert(handle);
	auto& theFile = files_[handle - 1];
	kassert(theFile.Tag == File::FILE_FS);
	return *theFile.FSFile.get();
}

size_t FileManager::GetFileSize(HANDLE file)
{
	return GetFSFile(file).DataLength;
}

void FileManager::ReadFile(HANDLE file, uint8_t* buffer, size_t offset, size_t length)
{

}