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
		void InitializeHeap(const BootParameters& bootParams) noexcept;
		void* HeapAlloc(size_t wantedSize) noexcept;
		void* HeapAlignedAlloc(size_t wantedSize, size_t alignment) noexcept;
		void HeapFree(void* ptr) noexcept;
		void HeapAlignedFree(void* ptr) noexcept;

		template<class T>
		struct aligned_heap_delete
		{
			void operator()(T* ptr) const noexcept { Memory::HeapAlignedFree(ptr); }
		};

		class MemoryManager
		{
		public:
			size_t GetFreeBytesRemaining() const noexcept;

			void* HeapAlloc(size_t wantedSize) noexcept { return Memory::HeapAlloc(wantedSize); }
			void* HeapAlignedAlloc(size_t wantedSize, size_t alignment) noexcept { return Memory::HeapAlignedAlloc(wantedSize, alignment); }
			void HeapFree(void* ptr) noexcept { Memory::HeapFree(ptr); }
			void HeapAlignedFree(void* ptr) noexcept { Memory::HeapAlignedFree(ptr); }
		};
	}

	template<class T>
	using aligned_heap_ptr = std::unique_ptr<T, Memory::aligned_heap_delete<T>>;
}

extern StaticHolder<Chino::Memory::MemoryManager> g_MemoryMgr;