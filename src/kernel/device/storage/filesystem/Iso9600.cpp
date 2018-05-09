//
// Kernel Device
//
#include "Iso9600.hpp"
#include "../../DeviceManager.hpp"
#include "../../../kdebug.hpp"
#include <portable.h>
#include <cstring>
#include <unordered_map>

using namespace Chino::Device;

DEFINE_PARTITION_DRIVER_DESC(Iso9600FileSystem);

struct PartitionTableDetector
{

};

Iso9600FileSystem::Iso9600FileSystem(Partition& partition)
	:partition_(partition)
{
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

		g_BootVideo->PutFormat(L"Volume(%d): Type: %s\n", (int)i, GetType(type));
		if (type == 255)break;

		if (type == 1)
		{
			primVolFound = true;
			pathTableLBA_ = *reinterpret_cast<const uint32_t*>(buffer.get() + 140);
			pathTableSize_ = *reinterpret_cast<const uint32_t*>(buffer.get() + 132);
			g_BootVideo->PutFormat(L"Path Table: LBA: %d, Size: %d\n", pathTableLBA_, pathTableSize_);
		}
	}

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

		paths[id] = { name, entry.ParentNumber };
		id++;

		g_BootVideo->PutFormat("Path: %s\n", name.c_str());
		return false;
	});
}

void Iso9600FileSystem::ForEachPathTable(std::function<bool(const PathEntry&)> callback)
{
	auto startLba = pathTableLBA_;
	auto buffer = std::make_unique<uint8_t[]>(partition_.BlockSize);

	BufferedBinaryReader br(buffer.get(), [&, this](uint8_t* buffer) {
		this->partition_.Read(startLba, 1, buffer);
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
