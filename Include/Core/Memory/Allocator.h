#pragma once

#include <Core/Header/Handle.h>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.h>

namespace CZ {

struct LinearAllocatorInfo {
    MemoryUsage Usage;
    size_t Capacity;
    bool bIsMultiPage;
};

struct LinearAllocator : Handle<struct LinearAllocatorObj> {
    static LinearAllocator Create(const LinearAllocatorInfo& info);
    static void Destroy(LinearAllocator allocator);
    void* Allocate(size_t size);
    void* AllocateAligned(size_t size, size_t alignment);
    void Free();
    size_t PageCount() const;
    size_t Capacity() const;
    size_t Size() const;
    size_t Remain() const;
};

struct PoolAllocatorInfo {
    MemoryUsage Usage;
    size_t BlockSize;
    size_t PageSize;
    bool bIsMultiPage;
};

struct PoolAllocator : Handle<struct PoolAllocatorObj> {
    static PoolAllocator Create(const PoolAllocatorInfo& info);
    static void Destroy(PoolAllocator allocator);
    void* Allocate();
    void Free(void* block);
    size_t PageCount() const;

    class Iterator {
    public:
        Iterator& operator++();
        inline bool operator==(const Iterator& other) { return m_Block == other.m_Block; }
        inline bool operator!=(const Iterator& other) { return m_Block != other.m_Block; }
        Iterator(Byte* page, Byte* block, size_t blocksLeft);
        inline void* Data() { return m_Block + 16; };
        inline operator bool() const { return m_Page != nullptr; }

    private:
        Byte* m_Page;
        Byte* m_Block;
        size_t m_BlocksLeft;
    };

    Iterator Begin();
};

} // namespace CZ