//
// Kernel Device
//
#include "Iso9600.hpp"
#include "../../DeviceManager.hpp"
#include "../../../kdebug.hpp"
#include <portable.h>
#include <cstring>

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
	kassert(partition_.BlockSize == 2048);

	// Primary Volume Descriptor
	for (size_t i = 0x10; i < partition_.MaxLBA; i++)
	{
		partition_.Read(i, 1, buffer);
		auto type = buffer[0];

		g_BootVideo->PutFormat(L"Volume(%d): Type: %s\n", (int)i, GetType(type));
		if (type == 255)break;
	}
}
