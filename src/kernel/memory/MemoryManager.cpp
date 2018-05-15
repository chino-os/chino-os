//
// Chino Memory
//
#include "MemoryManager.hpp"
#include "../kdebug.hpp"
#include <libbsp/bsp.hpp>

using namespace Chino::Memory;

void MemoryManager::InitializeHeap(const BootParameters& bootParams) noexcept
{
	BSPInitializeHeap(bootParams);
}

void* MemoryManager::HeapAlloc(size_t wantedSize) noexcept
{
	BSPHeapAlloc(wantedSize);
}

void* MemoryManager::HeapAlignedAlloc(size_t wantedSize, size_t alignment) noexcept
{
	auto offset = alignment - 1 + sizeof(void*);
	auto pHead = HeapAlloc(wantedSize + offset);
	if (pHead)
	{
		auto pReturn = (uintptr_t(pHead) + offset) & ~(alignment - 1);
		auto pLink = reinterpret_cast<void**>(pReturn - sizeof(void*));
		*pLink = pHead;
		return reinterpret_cast<void*>(pReturn);
	}

	return nullptr;
}

void MemoryManager::HeapFree(void* ptr) noexcept
{
	BSPHeapFree(ptr);
}

void MemoryManager::HeapAlignedFree(void* ptr) noexcept
{
	if (ptr)
	{
		auto puc = uintptr_t(ptr);
		auto pLink = reinterpret_cast<void**>(puc - sizeof(void*));
		HeapFree(*pLink);
	}
}

size_t MemoryManager::GetFreeBytesRemaining() const noexcept
{
	return BSPGetFreeBytesRemaining();
}