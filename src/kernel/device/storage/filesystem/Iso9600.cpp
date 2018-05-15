//
// Kernel Device
//
#include "Iso9600.hpp"
#include "../../../file/FileManager.hpp"
#include "../../../kdebug.hpp"
#include <cstring>
#include <unordered_map>
#include <optional>

using namespace Chino::Device;

DEFINE_PARTITION_DRIVER_DESC(Iso9600FileSystem);

struct PartitionTableDetector
{

};

Iso9600FileSystem::Iso9600FileSystem(Partition& partition)
	:partition_(partition)
{
	Name = "ISO 9600";
}

bool Iso9600FileSystem::IsSupported(Chino::Device::Partition& device)
{
	return true;
}

static wchar_t* types[] = {
	L"Boot Record",
	L"Primary Volume Descriptor",
	L"Supplementary Volume Descriptor",
	L"Volume Partition Descriptor",
	L"Reserved",
	L"Volume Descriptor Set Terminator"
};

static wchar_t* GetType(uint8_t type)
{
	if (type < 4)
		return types[type];
	else if (type == 255)
		return types[5];
	return types[4];
}

void Iso9600FileSystem::Install()
{
	auto buffer = std::make_unique<uint8_t[]>(partition_.BlockSize);
	bool primVolFound = false;
	// Primary Volume Descriptor
	for (size_t i = 0x10; i < partition_.MaxLBA; i++)
	{
		partition_.Read(i, 1, buffer.get());
		auto type = buffer[0];

		//g_Logger->PutFormat(L"Volume(%d): Type: %s\n", (int)i, GetType(type));
		if (type == 255)break;

		if (type == 1)
		{
			primVolFound = true;
			pathTableLBA_ = *reinterpret_cast<const uint32_t*>(buffer.get() + 140);
			pathTableSize_ = *reinterpret_cast<const uint32_t*>(buffer.get() + 132);
			BlockSize = *reinterpret_cast<const uint16_t*>(buffer.get() + 128);
			//g_Logger->PutFormat(L"Path Table: LBA: %d, Size: %d; BlockSize: %d\n", pathTableLBA_, pathTableSize_, (int)BlockSize);
		}
	}

	kassert(BlockSize == partition_.BlockSize);

	struct entry
	{
		std::string name;
		uint16_t parent;
	};

	kassert(primVolFound);

	uint16_t id = 1;
	std::unordered_map<uint16_t, entry> paths;
	ForEachPathTable([&, this](const PathEntry& entry)
	{
		std::string name;
		if (id == 1)
			name = "ROOT";
		else
			name = paths[entry.ParentNumber].name + "/" + std::string((char*)(entry.Identifier), entry.IdentifierLength);

		//g_Logger->PutFormat("<DIR>  %s\n", name.c_str());

		ForEachDirectoryEntry(entry.ExtentLBA, [&, this](const DirectoryEntry& dEntry)
		{
			if ((dEntry.FileFlags & 0b10) == 0)
			{
				auto ename = name + "/" + std::string((char*)(dEntry.IdentifierAndSystemUse), dEntry.IdentifierLength - 2);
				//g_Logger->PutFormat("<FILE> %s\n", ename.c_str());
			}
			return false;
		});

		paths[id] = { name, entry.ParentNumber };
		id++;
		return false;
	});

	g_FileMgr->RegisterFileSystems(*this);
}

void Iso9600FileSystem::ForEachPathTable(std::function<bool(const PathEntry&)> callback)
{
	auto startLba = pathTableLBA_;
	auto buffer = std::make_unique<uint8_t[]>(partition_.BlockSize);

	BufferedBinaryReader br(buffer.get(), [&, this](uint8_t* buffer) {
		this->partition_.Read(startLba++, 1, buffer);
		return this->partition_.BlockSize;
	});

	for (size_t i = 0; i < pathTableSize_;)
	{
		PathEntry entry{};
		auto head = reinterpret_cast<uint8_t*>(&entry);

		br.ReadBytes(head, 8);
		head += 8;
		auto idLen = entry.IdentifierLength;
		if (idLen % 2) idLen++;
		br.ReadBytes(head, idLen);
		if (callback(entry)) break;

		i += 8 + idLen;
	}
}

void Iso9600FileSystem::ForEachDirectoryEntry(uint32_t lba, std::function<bool(const DirectoryEntry&)> callback)
{
	auto buffer = std::make_unique<uint8_t[]>(partition_.BlockSize);
	partition_.Read(lba, 1, buffer.get());
	auto theDir = reinterpret_cast<DirectoryEntry*>(buffer.get());
	auto tableSize = theDir->DataLength;
	kassert(lba == theDir->ExtentLBA);

	BufferedBinaryReader br(buffer.get(), [&, this](uint8_t* buffer) {
		this->partition_.Read(lba++, 1, buffer);
		return this->partition_.BlockSize;
	}, partition_.BlockSize, 0);

	for (size_t i = 0; i < tableSize;)
	{
		DirectoryEntry entry{};
		auto head = reinterpret_cast<uint8_t*>(&entry);

		br.ReadBytes(head, 1);
		head += 1;
		if (entry.Length == 0)
		{
			i += 1 + br.AbandonBuffer();
		}
		else
		{
			br.ReadBytes(head, entry.Length - 1);
			if (callback(entry)) break;

			i += entry.Length;
		}
	}
}

struct Iso9600File : public FileSystemFile
{
	size_t DataLBA;

	using FileSystemFile::FileSystemFile;
};

std::unique_ptr<FileSystemFile> Iso9600FileSystem::TryOpenFile(const Path& filePath)
{
	for (auto comp : filePath)
	{
		std::string str(comp);
		g_Logger->PutFormat("C: %s ", str.c_str());
	}
	g_Logger->PutChar('\n');
	
	auto cntPathComp = filePath.begin();
	auto fileNameComp = --filePath.end();
	std::optional<uint32_t> pathLBA;
	uint16_t parentNumber = 1;
	uint16_t pathNumber = 0;
	ForEachPathTable([&, this](const PathEntry& entry)
	{
		pathNumber++;
		if (entry.ParentNumber < parentNumber) return false;
		if (entry.ParentNumber > parentNumber) return true;

		auto cmp = strncasecmp((char*)entry.Identifier, (*cntPathComp).data(), std::min(size_t(entry.IdentifierLength), (*cntPathComp).size()));
		if (cmp > 0) return true;

		if (cmp == 0 && (*cntPathComp).size() == entry.IdentifierLength)
		{
			if (++cntPathComp == fileNameComp)
			{
				pathLBA = entry.ExtentLBA;
				return true;
			}

			parentNumber = pathNumber;
			return false;
		}

		return false;
	});

	if (!pathLBA) return nullptr;
	
	std::optional<DirectoryEntry> fileEntry;
	ForEachDirectoryEntry(pathLBA.value(), [&, this](const DirectoryEntry& entry)
	{
		auto idLen = size_t(entry.IdentifierLength);
		if ((entry.FileFlags & 2) == 0) // Is a file
		{
			idLen -= 2;
			if (!filePath.Extension().size()) idLen--;
		}

		auto cmp = strncasecmp((char*)entry.IdentifierAndSystemUse, (*fileNameComp).data(), std::min(size_t(entry.IdentifierLength), (*fileNameComp).size()));
		if (cmp > 0) return true;

		if (cmp == 0 && (*fileNameComp).size() == idLen)
		{
			if ((entry.FileFlags & 2) == 0) // Is a file
				fileEntry = entry;
			return true;
		}

		return false;
	});

	if (!fileEntry) return nullptr;

	auto& entry = fileEntry.value();
	auto file = std::make_unique<Iso9600File>(*this);
	file->DataLBA = fileEntry.value().ExtentLBA;
	file->DataLength = fileEntry.value().DataLength;
	
	return std::move(file);
}

void Iso9600FileSystem::ReadFile(FileSystemFile& file, uint8_t* buffer, size_t blockOffset, size_t numBlocks)
{
	auto theFile = static_cast<Iso9600File&>(file);
	partition_.Read(theFile.DataLBA + blockOffset, numBlocks, buffer);
}
