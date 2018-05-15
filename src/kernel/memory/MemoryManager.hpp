//
// Chino Memory
//
#pragma once
#include "../kernel_iface.h"
#include "../utils.hpp"
#include <memory>

namespace Chino
{
	namespace Memory
	{
		template<class T>
		struct aligned_heap_delete
		{
			void operator()(T* ptr) const noexcept;
		};

		class MemoryManager
		{
		public:
			static void InitializeHeap(const BootParameters& bootParams) noexcept;

			size_t GetFreeBytesRemaining() const noexcept;

			void* HeapAlloc(size_t wantedSize) noexcept;
			void* HeapAlignedAlloc(size_t wantedSize, size_t alignment) noexcept;
			void HeapFree(void* ptr) noexcept;
			void HeapAlignedFree(void* ptr) noexcept;
		};
	}

	template<class T>
	using aligned_heap_ptr = std::unique_ptr<T, Memory::aligned_heap_delete<T>>;
}

extern StaticHolder<Chino::Memory::MemoryManager> g_MemoryMgr;

template<class T>
void Chino::Memory::aligned_heap_delete<T>::operator()(T* ptr) const noexcept
{
	g_MemoryMgr->HeapAlignedFree(ptr);
}
