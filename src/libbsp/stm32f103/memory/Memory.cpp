//
// Chino Memory
//
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>

using namespace Chino::Memory;

extern "C"
{
	extern size_t _heap_start, _heap_size;
}

bool Chino::Memory::BSPEnumHeapRegion(const BootParameters& bootParams, size_t index, HeapRegionDesc& desc)
{
	if (index == 0)
	{
		desc.StartAddress = _heap_start;
		desc.SizeInBytes = _heap_size;
		return true;
	}

	return false;
}
