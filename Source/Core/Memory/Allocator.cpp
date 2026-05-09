#include <Core/Header/Assert.h>
#include <Core/Header/Types.h>
#include <Core/Memory/Allocator.h>

namespace CZ {

struct LinearAllocatorObj {
    struct Page {
        Page* Next;
        size_t Used;
    };

    size_t Capacity;   /// byte capacity per page
    Page* PageList;    /// memory pages
    MemoryUsage Usage; /// usage domain
    bool bIsMultiPage; /// whether allocator paginates

    void AllocatePage() {
        Page* page = (Page*)HeapMalloc(sizeof(Page) + Capacity, Usage);
        page->Next = PageList;
        page->Used = 0;
        PageList   = page;
    }

    void Free() {
        Page* page = PageList;

        while (page) {
            Page* nextPage = page->Next;
            HeapFree(page);
            page = nextPage;
        }

        PageList = nullptr;
    }
};

LinearAllocator LinearAllocator::Create(const LinearAllocatorInfo& info) {
    LinearAllocatorObj* obj =
        (LinearAllocatorObj*)HeapMalloc(sizeof(LinearAllocatorObj), info.Usage);
    obj->Usage        = info.Usage;
    obj->Capacity     = info.Capacity;
    obj->PageList     = nullptr; // defer until first allocation
    obj->bIsMultiPage = info.bIsMultiPage;

    return { obj };
}

void LinearAllocator::Destroy(LinearAllocator allocator) {
    LinearAllocatorObj* obj = allocator;

    obj->Free();

    HeapFree(obj);
}

size_t LinearAllocator::PageCount() const {
    size_t count = 0;

    for (LinearAllocatorObj::Page* page = m_Obj->PageList; page; page = page->Next)
        count++;

    return count;
}

size_t LinearAllocator::Capacity() const { return m_Obj->Capacity; }

size_t LinearAllocator::Size() const {
    size_t size = 0;

    for (LinearAllocatorObj::Page* page = m_Obj->PageList; page; page = page->Next) {
        size += page->Used;
    }

    return size;
}

size_t LinearAllocator::Remain() const {
    LinearAllocatorObj::Page* currentPage = m_Obj->PageList;

    if (!currentPage || (m_Obj->bIsMultiPage && m_Obj->Capacity == currentPage->Used))
        return m_Obj->Capacity; // next page is available at max capacity

    return m_Obj->Capacity - currentPage->Used;
}

void* LinearAllocator::Allocate(size_t size) {
    if (size > m_Obj->Capacity) return nullptr; // can't satisfy request even in multi-page mode

    if (!m_Obj->PageList || (m_Obj->bIsMultiPage && Remain() < size)) m_Obj->AllocatePage();

    LinearAllocatorObj::Page* currentPage = m_Obj->PageList;
    CZ_CORE_ASSERT(currentPage);

    if (currentPage->Used + size <= m_Obj->Capacity) {
        Byte* base = (Byte*)currentPage + sizeof(LinearAllocatorObj::Page) + currentPage->Used;
        currentPage->Used += size;
        return base;
    }

    return nullptr;
}

void* LinearAllocator::AllocateAligned(size_t size, size_t alignment) {
    if (!m_Obj->PageList) m_Obj->AllocatePage();

    LinearAllocatorObj::Page* currentPage = m_Obj->PageList;
    CZ_CORE_ASSERT(currentPage);

    uintptr_t now =
        (uintptr_t)((Byte*)currentPage + sizeof(LinearAllocatorObj::Page) + currentPage->Used);
    uintptr_t misalignment = now & (alignment - 1);
    uintptr_t pad          = (misalignment == 0) ? 0 : (alignment - misalignment);

    if (pad + size > m_Obj->Capacity) return nullptr;

    if (pad + size > Remain()) m_Obj->AllocatePage();

    currentPage = m_Obj->PageList;
    CZ_CORE_ASSERT(currentPage->used + pad + size <= m_Obj->capacity);

    Byte* base = (Byte*)currentPage + sizeof(LinearAllocatorObj::Page) + currentPage->Used + pad;
    currentPage->Used += pad + size;

    CZ_CORE_ASSERT(((uintptr_t)base) % alignment == 0);
    return base;
}

void LinearAllocator::Free() { m_Obj->Free(); }

struct PoolAllocatorObj {
    struct Block;
    struct Page;

    struct Block {
        Block* Next; /// next block free for allocation
        Page* Owner; /// null if the block is free, otherwise the memory page this block belongs to

        inline bool isAllocated() const { return Owner != nullptr; }
    };

    struct Page {
        PoolAllocatorObj* Obj; /// pool allocator object
        Page* Next;            /// linked list of memory pages
        Block* FreeBlocks;     /// linked list of blocks free for allocation
        size_t FreeBlockCount; /// length of freeBlocks linked list

        Block* GetFirstAllocatedBlock() {
            if (FreeBlockCount == Obj->PageSize) return nullptr;

            Block* block = (Block*)(this + 1);

            for (size_t i = 0; i < Obj->PageSize; i++) {
                if (block->isAllocated()) return block;

                block = (PoolAllocatorObj::Block*)((Byte*)block + Obj->BlockSize);
            }

            return nullptr;
        }

        inline size_t AllocatedBlockCount() { return Obj->PageSize - FreeBlockCount; }
    };

    size_t BlockSize;
    size_t PageSize;
    Page* PageList;
    MemoryUsage Usage;
    bool bIsMultiPage;

    void AllocatePage() {
        Page* page           = (Page*)HeapMalloc(sizeof(Page) + BlockSize * PageSize, Usage);
        page->Obj            = this;
        page->Next           = PageList;
        PageList             = page;
        page->FreeBlocks     = (Block*)(page + 1);
        page->FreeBlockCount = PageSize;
        Block* block         = page->FreeBlocks;

        for (size_t i = 0; i < PageSize - 1; i++) {
            block->Next  = (Block*)((Byte*)block + BlockSize);
            block->Owner = nullptr;
            block        = block->Next;
        }

        block->Owner = nullptr;
        block->Next  = nullptr;
    }
};

static_assert(sizeof(PoolAllocatorObj::Block) == 16); // update Allocator.h

PoolAllocator PoolAllocator::Create(const PoolAllocatorInfo& info) {
    CZ_CORE_ASSERT(info.BlockSize != 0 && info.PageSize > 0);

    PoolAllocatorObj* obj = (PoolAllocatorObj*)HeapMalloc(sizeof(PoolAllocatorObj), info.Usage);
    obj->Usage            = info.Usage;
    obj->BlockSize = info.BlockSize +
                     sizeof(PoolAllocatorObj::Block); // each block includes 16-byte header overhead
    obj->PageSize  = info.PageSize;
    obj->bIsMultiPage = info.bIsMultiPage;
    obj->PageList     = nullptr; // defer until first allocation

    return { obj };
}

void PoolAllocator::Destroy(PoolAllocator allocator) {
    PoolAllocatorObj* obj = allocator;

    while (obj->PageList) {
        PoolAllocatorObj::Page* page = obj->PageList;
        obj->PageList                = obj->PageList->Next;
        HeapFree(page);
    }

    HeapFree(obj);
}

void* PoolAllocator::Allocate() {
    if (!m_Obj->PageList) m_Obj->AllocatePage();

    for (PoolAllocatorObj::Page* page = m_Obj->PageList; page; page = page->Next) {
        if (page->FreeBlocks) {
            CZ_CORE_ASSERT(page->FreeBlockCount > 0);
            PoolAllocatorObj::Block* blk = page->FreeBlocks;
            page->FreeBlocks             = page->FreeBlocks->Next;
            page->FreeBlockCount--;
            blk->Owner = page;
            return blk + 1;
        }
    }

    if (m_Obj->bIsMultiPage) {
        m_Obj->AllocatePage();
        PoolAllocatorObj::Page* page = m_Obj->PageList;
        CZ_CORE_ASSERT(page && page->FreeBlocks);

        PoolAllocatorObj::Block* blk = page->FreeBlocks;
        page->FreeBlocks             = page->FreeBlocks->Next;
        page->FreeBlockCount--;
        blk->Owner = page;
        return blk + 1;
    }

    // out of blocks in single page mode
    return nullptr;
}

void PoolAllocator::Free(void* block) {
    PoolAllocatorObj::Block* blk = (PoolAllocatorObj::Block*)block - 1;
    PoolAllocatorObj::Page* page = blk->Owner;

    CZ_CORE_ASSERT(page);

    // return block to owning page
    blk->Next        = page->FreeBlocks;
    blk->Owner       = nullptr;
    page->FreeBlocks = blk;
    page->FreeBlockCount++;
}

size_t PoolAllocator::PageCount() const {
    size_t count = 0;

    for (PoolAllocatorObj::Page* page = m_Obj->PageList; page; page = page->Next)
        count++;

    return count;
}

PoolAllocator::Iterator& PoolAllocator::Iterator::operator++() {
    // jump to next page or return end
    if (m_BlocksLeft == 0) {
        for (auto* page = ((PoolAllocatorObj::Page*)m_Page)->Next; page; page = page->Next) {
            PoolAllocatorObj::Block* block = page->GetFirstAllocatedBlock();

            if (block) {
                m_Page       = (Byte*)page;
                m_Block      = (Byte*)block;
                m_BlocksLeft = page->AllocatedBlockCount() - 1;
                return *this;
            }
        }

        // complete iteration
        m_Page  = nullptr;
        m_Block = nullptr;
        return *this;
    }

    auto* obj   = (PoolAllocatorObj*)((PoolAllocatorObj::Page*)m_Page)->Obj;
    auto* block = (PoolAllocatorObj::Block*)m_Block;

    do {
        block = (PoolAllocatorObj::Block*)((Byte*)block + obj->BlockSize);
    } while (!block->isAllocated());

    m_BlocksLeft--;
    m_Block = (Byte*)block;
    return *this;
}

PoolAllocator::Iterator::Iterator(Byte* page, Byte* block, size_t blocksLeft)
    : m_Page(page), m_Block(block), m_BlocksLeft(blocksLeft) {}

PoolAllocator::Iterator PoolAllocator::Begin() {
    for (PoolAllocatorObj::Page* page = m_Obj->PageList; page; page = page->Next) {
        PoolAllocatorObj::Block* block = page->GetFirstAllocatedBlock();

        if (block) {
            CZ_CORE_ASSERT(m_Obj->PageSize > page->FreeBlockCount);
            return Iterator((Byte*)page, (Byte*)block, page->AllocatedBlockCount() - 1);
        }
    }

    return Iterator(nullptr, nullptr, 0);
}

} // namespace CZ