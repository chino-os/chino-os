//
// Chino Memory
//
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>

using namespace Chino::Memory;

static volatile int i = 1028;

bool Chino::Memory::BSPEnumHeapRegion(const BootParameters& bootParams, size_t index, HeapRegionDesc& desc)
{
	//g_Logger->PutFormat("Heap Start: %x, End: %x\n", uintptr_t(&_heap_start[0]), uintptr_t(&_heap_end[0]));

	if (i != 1028)
		g_Logger->PutChar('F');

	if (index == 0)
	{
		desc.StartAddress = uintptr_t(0x3F800000);
		desc.SizeInBytes = size_t(0x400000);
		return true;
	}

	return false;
}
