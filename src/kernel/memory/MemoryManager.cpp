//
// Chino Memory
//
#include "MemoryManager.hpp"
#include "../kdebug.hpp"

using namespace Chino::Memory;

#define traceMALLOC(p, size)
#define traceFREE(p, size)

#define portBYTE_ALIGNMENT	8

#if portBYTE_ALIGNMENT == 32
#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
#define portBYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
#define portBYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
#define portBYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef portBYTE_ALIGNMENT_MASK
#error "Invalid portBYTE_ALIGNMENT definition"
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( heapStructSize_ << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

typedef struct A_BLOCK_LINK
{
	A_BLOCK_LINK *pNextFreeBlock;	/*<< The next free block in the list. */
	size_t BlockSize;				/*<< The size of the free block. */
} BlockLink_t;

/*
* Inserts a block of memory that is being freed into the correct position in
* the list of free memory blocks.  The block being freed will be merged with
* the block in front it and/or the block behind it if the memory blocks are
* adjacent to each other.
*/
static void prvInsertBlockIntoFreeList(BlockLink_t *pBlockToInsert);

/*-----------------------------------------------------------*/
BlockLink_t heapStart_, *pHeapEnd_ = nullptr;
size_t freeBytesRemaining_ = 0;
size_t minimumEverFreeBytesRemaining_ = 0;
size_t blockAllocatedBit_ = 0;
/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static constexpr size_t heapStructSize_ = (sizeof(BlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) & ~((size_t)portBYTE_ALIGNMENT_MASK);

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

void Chino::Memory::InitializeHeap(const BootParameters& bootParams) noexcept
{
	BlockLink_t *pFirstFreeBlockInRegion = nullptr, *pPreviousFreeBlock;
	uintptr_t alignedHeap;
	size_t totalRegionSize, totalHeapSize = 0;
	size_t definedRegions = 0;
	uintptr_t address;
	auto pHeapRegion = bootParams.EfiMemoryDescriptor;

	const auto totalRegions = bootParams.EfiMemoryDescriptorCount;
	const auto descriptorSize = bootParams.EfiMemoryDescriptorSize;

	/* Can only call once! */
	kassert(pHeapEnd_ == nullptr);
	for (size_t i = 0; i < totalRegions; i++)
	{
		if (IsAvailableRegion(pHeapRegion))
		{
			totalRegionSize = pHeapRegion->NumberOfPages << EFI_PAGE_SHIFT;
			address = pHeapRegion->PhysicalStart;
			/* Ensure the heap region starts on a correctly aligned boundary. */
			if ((address & portBYTE_ALIGNMENT_MASK) != 0)
			{
				address += (portBYTE_ALIGNMENT - 1);
				address &= ~portBYTE_ALIGNMENT_MASK;

				/* Adjust the size for the bytes lost to alignment. */
				totalRegionSize -= address - (size_t)pHeapRegion->PhysicalStart;
			}

			alignedHeap = address;

			/* Set xStart if it has not already been set. */
			if (definedRegions == 0)
			{
				/* xStart is used to hold a pointer to the first item in the list of
				free blocks.  The void cast is used to prevent compiler warnings. */
				heapStart_.pNextFreeBlock = reinterpret_cast<BlockLink_t*>(alignedHeap);
				heapStart_.BlockSize = 0;
			}
			else
			{
				/* Should only get here if one region has already been added to the
				heap. */
				kassert(pHeapEnd_);

				/* Check blocks are passed in with increasing start addresses. */
				kassert(address > uintptr_t(pHeapEnd_));
			}

			/* Remember the location of the end marker in the previous region, if
			any. */
			pPreviousFreeBlock = pHeapEnd_;

			/* pxEnd is used to mark the end of the list of free blocks and is
			inserted at the end of the region space. */
			address = alignedHeap + totalRegionSize;
			address -= heapStructSize_;
			address &= ~portBYTE_ALIGNMENT_MASK;
			pHeapEnd_ = reinterpret_cast<BlockLink_t*>(address);
			pHeapEnd_->BlockSize = 0;
			pHeapEnd_->pNextFreeBlock = nullptr;

			/* To start with there is a single free block in this region that is
			sized to take up the entire heap region minus the space taken by the
			free block structure. */
			pFirstFreeBlockInRegion = reinterpret_cast<BlockLink_t*>(alignedHeap);
			pFirstFreeBlockInRegion->BlockSize = address - uintptr_t(pFirstFreeBlockInRegion);
			pFirstFreeBlockInRegion->pNextFreeBlock = pHeapEnd_;

			/* If this is not the first region that makes up the entire heap space
			then link the previous region to this region. */
			if (pPreviousFreeBlock)
				pPreviousFreeBlock->pNextFreeBlock = pFirstFreeBlockInRegion;

			totalHeapSize += pFirstFreeBlockInRegion->BlockSize;

			/* Move onto the next HeapRegion_t structure. */
			definedRegions++;
		}

		pHeapRegion = NextMemoryDescriptor(pHeapRegion, descriptorSize);
	}

	minimumEverFreeBytesRemaining_ = totalHeapSize;
	freeBytesRemaining_ = totalHeapSize;

	/* Check something was actually defined before it is accessed. */
	kassert(totalHeapSize);

	/* Work out the position of the top bit in a size_t variable. */
	blockAllocatedBit_ = ((size_t)1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);
}

void* Chino::Memory::HeapAlloc(size_t wantedSize) noexcept
{
	BlockLink_t *pBlock, *pPreviousBlock, *pNewBlockLink;
	void *pReturn = nullptr;

	/* The heap must be initialised before the first call to
	prvPortMalloc(). */
	kassert(pHeapEnd_);

	//vTaskSuspendAll();
	{
		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if ((wantedSize & blockAllocatedBit_) == 0)
		{
			/* The wanted size is increased so it can contain a BlockLink_t
			structure in addition to the requested amount of bytes. */
			if (wantedSize > 0)
			{
				wantedSize += heapStructSize_;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if ((wantedSize & portBYTE_ALIGNMENT_MASK) != 0x00)
				{
					/* Byte alignment required. */
					wantedSize += (portBYTE_ALIGNMENT - (wantedSize & portBYTE_ALIGNMENT_MASK));
				}
			}

			if ((wantedSize > 0) && (wantedSize <= freeBytesRemaining_))
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pPreviousBlock = &heapStart_;
				pBlock = heapStart_.pNextFreeBlock;
				while ((pBlock->BlockSize < wantedSize) && pBlock->pNextFreeBlock)
				{
					pPreviousBlock = pBlock;
					pBlock = pBlock->pNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if (pBlock != pHeapEnd_)
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pReturn = reinterpret_cast<void*>(uintptr_t(pPreviousBlock->pNextFreeBlock) + heapStructSize_);

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pPreviousBlock->pNextFreeBlock = pBlock->pNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if ((pBlock->BlockSize - wantedSize) > heapMINIMUM_BLOCK_SIZE)
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pNewBlockLink = reinterpret_cast<BlockLink_t*>(uintptr_t(pBlock) + wantedSize);

						/* Calculate the sizes of two blocks split from the
						single block. */
						pNewBlockLink->BlockSize = pBlock->BlockSize - wantedSize;
						pBlock->BlockSize = wantedSize;

						/* Insert the new block into the list of free blocks. */
						prvInsertBlockIntoFreeList(pNewBlockLink);
					}

					freeBytesRemaining_ -= pBlock->BlockSize;

					if (freeBytesRemaining_ < minimumEverFreeBytesRemaining_)
						minimumEverFreeBytesRemaining_ = freeBytesRemaining_;

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pBlock->BlockSize |= blockAllocatedBit_;
					pBlock->pNextFreeBlock = nullptr;
				}
			}
		}

		traceMALLOC(pvReturn, xWantedSize);
	}
	//(void)xTaskResumeAll();

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if (pvReturn == NULL)
		{
			extern void vApplicationMallocFailedHook(void);
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
#endif

	return pReturn;
}

void Chino::Memory::HeapFree(void* ptr) noexcept
{
	auto puc = uintptr_t(ptr);
	BlockLink_t *pLink;

	if (ptr)
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= heapStructSize_;

		/* This casting is to keep the compiler from issuing warnings. */
		pLink = reinterpret_cast<BlockLink_t*>(puc);

		/* Check the block is actually allocated. */
		kassert(pLink->BlockSize & blockAllocatedBit_);
		kassert(pLink->pNextFreeBlock == nullptr);

		if (pLink->BlockSize & blockAllocatedBit_)
		{
			if (pLink->pNextFreeBlock == nullptr)
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pLink->BlockSize &= ~blockAllocatedBit_;

				//vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					freeBytesRemaining_ += pLink->BlockSize;
					traceFREE(pv, pxLink->xBlockSize);
					prvInsertBlockIntoFreeList(pLink);
				}
				//(void)xTaskResumeAll();
			}
		}
	}
}

static void prvInsertBlockIntoFreeList(BlockLink_t *pBlockToInsert)
{
	BlockLink_t *pIterator;
	uintptr_t puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for (pIterator = &heapStart_; pIterator->pNextFreeBlock < pBlockToInsert; pIterator = pIterator->pNextFreeBlock)
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = uintptr_t(pIterator);
	if ((puc + pIterator->BlockSize) == uintptr_t(pBlockToInsert))
	{
		pIterator->BlockSize += pBlockToInsert->BlockSize;
		pBlockToInsert = pIterator;
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = uintptr_t(pBlockToInsert);
	if ((puc + pBlockToInsert->BlockSize) == uintptr_t(pIterator->pNextFreeBlock))
	{
		if (pIterator->pNextFreeBlock != pHeapEnd_)
		{
			/* Form one big block from the two blocks. */
			pBlockToInsert->BlockSize += pIterator->pNextFreeBlock->BlockSize;
			pBlockToInsert->pNextFreeBlock = pIterator->pNextFreeBlock->pNextFreeBlock;
		}
		else
		{
			pBlockToInsert->pNextFreeBlock = pHeapEnd_;
		}
	}
	else
	{
		pBlockToInsert->pNextFreeBlock = pIterator->pNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if (pIterator != pBlockToInsert)
	{
		pIterator->pNextFreeBlock = pBlockToInsert;
	}
}

size_t MemoryManager::GetFreeBytesRemaining() const noexcept
{
	return freeBytesRemaining_;
}