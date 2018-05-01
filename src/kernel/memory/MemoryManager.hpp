//
// Chino Memory
//
#pragma once
#include "../kernel_iface.h"

namespace Chino
{
	namespace Memory
	{
		class MemoryManager
		{
		public:
			size_t GetFreeBytesRemaining() const noexcept;
		};

		void InitializeHeap(const BootParameters& bootParams) noexcept;
		void* HeapAlloc(size_t wantedSize) noexcept;
		void HeapFree(void* ptr) noexcept;
	}
}