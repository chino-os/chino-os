//
// Chino Memory
//
#include "../bsp_defines.hpp"
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>

using namespace Chino::Memory;

static bool IsAvailableRegion(const EFI_MEMORY_DESCRIPTOR* region)
{
	switch (region->Type)
	{
	case EfiConventionalMemory:
	case EfiLoaderCode:
	case EfiLoaderData:
	case EfiBootServicesCode:
	case EfiBootServicesData:
		return true;
	default:
		return false;
	}
}

bool Chino::Memory::BSPEnumHeapRegion(const BootParameters& bootParams, size_t index, HeapRegionDesc& desc)
{
	const auto totalRegions = bootParams.Efi.MemoryDescriptorCount;
	const auto descriptorSize = bootParams.Efi.MemoryDescriptorSize;
	auto pHeapRegion = bootParams.Efi.MemoryDescriptor;

	size_t cntIndx = 0;
	for (size_t i = 0; i < totalRegions; i++)
	{
		if (IsAvailableRegion(pHeapRegion))
		{
			if (index == cntIndx)
			{
				desc.StartAddress = pHeapRegion->PhysicalStart;
				desc.SizeInBytes = pHeapRegion->NumberOfPages << EFI_PAGE_SHIFT;
				return true;
			}

			pHeapRegion = NextMemoryDescriptor(pHeapRegion, descriptorSize);
			cntIndx++;
		}
	}

	return false;
}
