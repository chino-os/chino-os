//
// Chino Memory
//
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>

using namespace Chino::Memory;

extern "C"
{
	extern char _heap_start[];
	extern char _heap_end[];
}

bool Chino::Memory::BSPEnumHeapRegion(const BootParameters& bootParams, size_t index, HeapRegionDesc& desc)
{
	//g_Logger->PutFormat("Heap Start: %x, End: %x\n", uintptr_t(&_heap_start[0]), uintptr_t(&_heap_end[0]));

	if (index == 0)
	{
		desc.StartAddress = uintptr_t(&_heap_start[0]);
		desc.SizeInBytes = size_t(&_heap_end[0] - &_heap_start[0]);
		return true;
	}

	return false;
}
