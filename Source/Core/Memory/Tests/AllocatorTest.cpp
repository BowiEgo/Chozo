#include <Core/Memory/Allocator.h>

#include <doctest/doctest.h>

#include <vector>

using namespace CZ;

TEST_SUITE("LinearAllocator") {

    TEST_CASE("Create and destroy - basic") {
        LinearAllocatorInfo info{ MEMORY_USAGE_MISC, 1024, false };
        LinearAllocator alloc = LinearAllocator::Create(info);
        // Verify creation succeeded using only public methods
        CHECK(alloc.Capacity() == 1024);
        CHECK(alloc.Size() == 0);
        CHECK(alloc.Remain() == 1024);
        CHECK(alloc.PageCount() == 0); // deferred page allocation
        LinearAllocator::Destroy(alloc);
    }

    TEST_CASE("Single page allocation") {
        LinearAllocatorInfo info{ MEMORY_USAGE_MISC, 1024, false };
        LinearAllocator alloc = LinearAllocator::Create(info);

        void* p1 = alloc.Allocate(128);
        REQUIRE(p1 != nullptr);
        CHECK(alloc.Size() == 128);
        CHECK(alloc.Remain() == 1024 - 128);
        CHECK(alloc.PageCount() == 1);

        void* p2 = alloc.Allocate(256);
        REQUIRE(p2 != nullptr);
        CHECK(alloc.Size() == 128 + 256);
        CHECK(alloc.Remain() == 1024 - 128 - 256);

        // Request larger than remaining capacity should fail
        void* p3 = alloc.Allocate(1000);
        REQUIRE(p3 == nullptr);
        CHECK(alloc.Size() == 128 + 256); // unchanged

        LinearAllocator::Destroy(alloc);
    }

    TEST_CASE("Multi-page allocation") {
        LinearAllocatorInfo info{ MEMORY_USAGE_SCENE, 256, true }; // 256 bytes per page
        LinearAllocator alloc = LinearAllocator::Create(info);

        void* p1 = alloc.Allocate(200);
        REQUIRE(p1 != nullptr);
        CHECK(alloc.PageCount() == 1);
        CHECK(alloc.Size() == 200);
        CHECK(alloc.Remain() == 56);

        // Allocating 100 bytes exceeds the remaining of current page, triggers a new page
        void* p2 = alloc.Allocate(100);
        REQUIRE(p2 != nullptr);
        CHECK(alloc.PageCount() == 2);
        CHECK(alloc.Size() == 200 + 100);
        CHECK(alloc.Remain() == 256 - 100);

        // Even in multi-page mode, requesting more than page capacity should fail
        void* p3 = alloc.Allocate(300);
        REQUIRE(p3 == nullptr);

        LinearAllocator::Destroy(alloc);
    }

    TEST_CASE("Aligned allocation") {
        LinearAllocatorInfo info{ MEMORY_USAGE_MISC, 1024, false };
        LinearAllocator alloc = LinearAllocator::Create(info);

        void* p1 = alloc.AllocateAligned(200, 64);
        REQUIRE(p1 != nullptr);
        CHECK(((uintptr_t)p1) % 64 == 0);
        CHECK(alloc.Size() > 0);

        // There should still be remaining space
        void* p2 = alloc.Allocate(128);
        REQUIRE(p2 != nullptr);
        CHECK(alloc.Size() >= 200 + 128); // alignment may cause internal padding

        LinearAllocator::Destroy(alloc);
    }

    TEST_CASE("Free and reuse") {
        LinearAllocatorInfo info{ MEMORY_USAGE_ASSET, 512, true };
        LinearAllocator alloc = LinearAllocator::Create(info);

        void* p = alloc.Allocate(128);
        REQUIRE(p != nullptr);
        CHECK(alloc.PageCount() == 1);

        alloc.Free(); // frees all pages
        CHECK(alloc.PageCount() == 0);
        CHECK(alloc.Size() == 0);
        CHECK(alloc.Remain() == 512); // back to full capacity

        void* p2 = alloc.Allocate(256);
        REQUIRE(p2 != nullptr);
        CHECK(alloc.PageCount() == 1); // new page

        LinearAllocator::Destroy(alloc);
    }
}

TEST_SUITE("PoolAllocator") {

    TEST_CASE("Create and destroy") {
        PoolAllocatorInfo info{ MEMORY_USAGE_UI, 64, 10, false };
        PoolAllocator pool = PoolAllocator::Create(info);
        CHECK(pool.PageCount() == 0); // deferred allocation
        PoolAllocator::Destroy(pool);
    }

    TEST_CASE("Single page allocate and deallocate") {
        PoolAllocatorInfo info{ MEMORY_USAGE_MISC, 32, 5, false };
        PoolAllocator pool = PoolAllocator::Create(info);

        void* blocks[5];
        for (int i = 0; i < 5; ++i) {
            blocks[i] = pool.Allocate();
            REQUIRE(blocks[i] != nullptr);
        }
        CHECK(pool.PageCount() == 1);

        // In single page mode, further allocation should fail
        void* extra = pool.Allocate();
        REQUIRE(extra == nullptr);

        // Free two blocks
        pool.Free(blocks[0]);
        pool.Free(blocks[2]);

        // Allocate twice again, should reuse the just freed blocks
        void* b1 = pool.Allocate();
        REQUIRE(b1 != nullptr);
        void* b2 = pool.Allocate();
        REQUIRE(b2 != nullptr);
        // After these, the pool should be full again
        REQUIRE(pool.Allocate() == nullptr);

        PoolAllocator::Destroy(pool);
    }

    TEST_CASE("Multi-page expansion") {
        PoolAllocatorInfo info{ MEMORY_USAGE_RENDER, 64, 3, true }; // 3 blocks per page
        PoolAllocator pool = PoolAllocator::Create(info);

        // Fill the first page
        void* a1 = pool.Allocate();
        void* a2 = pool.Allocate();
        void* a3 = pool.Allocate();
        REQUIRE(a1);
        REQUIRE(a2);
        REQUIRE(a3);
        CHECK(pool.PageCount() == 1);

        void* a4 = pool.Allocate();
        REQUIRE(a4 != nullptr);
        CHECK(pool.PageCount() == 2);

        pool.Free(a2);
        void* a5 = pool.Allocate();
        CHECK(pool.PageCount() == 2); // page count unchanged

        void* a6 = pool.Allocate();
        void* a7 = pool.Allocate();
        REQUIRE(a7 == a2);            // reused the same address
        CHECK(pool.PageCount() == 2); // page count unchanged

        PoolAllocator::Destroy(pool);
    }

    TEST_CASE("Iterator traversal") {
        PoolAllocatorInfo info{ MEMORY_USAGE_SCENE, 128, 8, true };
        PoolAllocator pool = PoolAllocator::Create(info);

        std::vector<void*> ptrs;
        for (int i = 0; i < 5; ++i) {
            ptrs.push_back(pool.Allocate());
        }

        // Iterate allocated blocks
        int count = 0;
        for (auto it = pool.Begin(); it; ++it) {
            void* block = it.Data();
            REQUIRE(block != nullptr);
            CHECK(std::find(ptrs.begin(), ptrs.end(), block) != ptrs.end());
            ++count;
        }
        CHECK(count == 5);

        // Free two blocks, iteration count should decrease
        pool.Free(ptrs[0]);
        pool.Free(ptrs[2]);
        count = 0;
        for (auto it = pool.Begin(); it; ++it) {
            ++count;
        }
        CHECK(count == 3); // 5 - 2

        PoolAllocator::Destroy(pool);
    }
}