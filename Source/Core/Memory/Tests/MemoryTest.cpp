// #include <Core/Memory/Memory.h>

// #include <doctest/doctest.h>

// #include <cstring>
// #include <string>
// #include <vector>

// using namespace CZ;

// // Helper macro: assert that the current allocation for a specific usage is zero
// #define REQUIRE_ZERO(usage) REQUIRE_EQ(GetMemoryProfile(usage).Current, 0)

// // Helper class: records initial value on construction, optionally checks on destruction
// struct ScopedMemoryGuard {
//     MemoryUsage usage;
//     std::size_t initial;
//     ScopedMemoryGuard(MemoryUsage u) : usage(u), initial(GetMemoryProfile(u).Current) {}
//     ~ScopedMemoryGuard() {
//         // No forced output, only for debugging assistance
//     }
// };

// TEST_SUITE("HeapMalloc / HeapFree") {

//     TEST_CASE("Basic allocation and free") {
//         constexpr size_t size = 128;
//         void* ptr             = HEAP_MALLOC(size, MEMORY_USAGE_MISC);
//         REQUIRE(ptr != nullptr);

//         const auto& prof = GetMemoryProfile(MEMORY_USAGE_MISC);
//         REQUIRE_EQ(prof.Current, size);
//         REQUIRE_GE(prof.Peak, size);

//         HeapFree(ptr);
//         REQUIRE_ZERO(MEMORY_USAGE_MISC);
//         REQUIRE_GE(GetMemoryProfile(MEMORY_USAGE_MISC).Peak, size);
//     }

//     TEST_CASE("Multiple allocations across usages") {
//         void* a = HEAP_MALLOC(32, MEMORY_USAGE_RENDER);
//         void* b = HEAP_MALLOC(48, MEMORY_USAGE_ASSET);
//         REQUIRE(a);
//         REQUIRE(b);

//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_RENDER).Current, 32);
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_ASSET).Current, 48);

//         HeapFree(a);
//         REQUIRE_ZERO(MEMORY_USAGE_RENDER);
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_ASSET).Current, 48); // unchanged

//         HeapFree(b);
//         REQUIRE_ZERO(MEMORY_USAGE_ASSET);
//     }
// }

// TEST_SUITE("HeapStrdup") {

//     TEST_CASE("Duplicate string and free") {
//         const char* original = "Hello, Chozo!";
//         char* copy           = HeapStrdup(original, MEMORY_USAGE_SERIAL);
//         REQUIRE(copy != nullptr);
//         CHECK_EQ(std::strcmp(original, copy), 0);

//         const size_t expectedSize = std::strlen(original) + 1;
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_SERIAL).Current, expectedSize);

//         HeapFree(copy);
//         REQUIRE_ZERO(MEMORY_USAGE_SERIAL);
//     }
// }

// TEST_SUITE("MemoryProfile and leak detection") {

//     TEST_CASE("Peak tracking") {
//         REQUIRE_ZERO(MEMORY_USAGE_UI); // start clean

//         void* a = HEAP_MALLOC(64, MEMORY_USAGE_UI);
//         void* b = HEAP_MALLOC(256, MEMORY_USAGE_UI);
//         REQUIRE(a);
//         REQUIRE(b);

//         const auto& prof = GetMemoryProfile(MEMORY_USAGE_UI);
//         CHECK_EQ(prof.Current, 64 + 256);
//         CHECK_GE(prof.Peak, 64 + 256);

//         HeapFree(b);
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_UI).Current, 64);
//         CHECK_GE(GetMemoryProfile(MEMORY_USAGE_UI).Peak, 64 + 256); // peak does not drop

//         HeapFree(a);
//         REQUIRE_ZERO(MEMORY_USAGE_UI);
//     }

//     TEST_CASE("Leak detection") {
//         // intentionally leak one block
//         REQUIRE_ZERO(MEMORY_USAGE_SCENE);
//         void* leak = HEAP_MALLOC(100, MEMORY_USAGE_SCENE);
//         REQUIRE(leak);

//         MemoryProfile leaks[10];
//         int count = GetMemoryLeaks(leaks);
//         REQUIRE_GE(count, 1);

//         bool found = false;
//         for (int i = 0; i < count; ++i) {
//             if (leaks[i].Current == 100 && leaks[i].Usage == MEMORY_USAGE_SCENE) {
//                 found = true;
//                 break;
//             }
//         }
//         CHECK(found);

//         // clean up leak
//         HeapFree(leak);
//         REQUIRE_ZERO(MEMORY_USAGE_SCENE);
//     }

//     TEST_CASE("GetMemoryUsageCstr") {
//         CHECK(std::strcmp(GetMemoryUsageCstr(MEMORY_USAGE_MISC), "MEMORY_USAGE_MISC") == 0);
//         CHECK(std::strcmp(GetMemoryUsageCstr(MEMORY_USAGE_RENDER), "MEMORY_USAGE_RENDER") == 0);
//     }
// }

// TEST_SUITE("New / Delete template functions") {

//     struct TestObj {
//         int value;
//         TestObj(int v) : value(v) {}
//     };

//     TEST_CASE("New and Delete with usage") {
//         REQUIRE_ZERO(MEMORY_USAGE_JOB_SYSTEM);
//         auto* obj = CZ_NEW(MEMORY_USAGE_JOB_SYSTEM, TestObj, 77);
//         REQUIRE(obj != nullptr);
//         CHECK_EQ(obj->value, 77);

//         // memory stat should equal sizeof(TestObj)
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_JOB_SYSTEM).Current, sizeof(TestObj));

//         Delete(obj);
//         REQUIRE_ZERO(MEMORY_USAGE_JOB_SYSTEM);
//     }

//     TEST_CASE("New without usage (uses MemoryTraits)") {
//         // default MemoryTraits<TestObj>::Usage is MEMORY_USAGE_MISC
//         REQUIRE_ZERO(MEMORY_USAGE_MISC);
//         auto* obj = CZ_NEW(TestObj,99);
//         REQUIRE(obj != nullptr);
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_MISC).Current, sizeof(TestObj));

//         Delete(obj);
//         REQUIRE_ZERO(MEMORY_USAGE_MISC);
//     }

//     TEST_CASE("Exception safety in New") {
//         struct ThrowOnConstruct {
//             ThrowOnConstruct() { throw std::runtime_error("fail"); }
//         };

//         auto before = GetMemoryProfile(MEMORY_USAGE_MISC).Current;
//         REQUIRE_THROWS_AS(New<ThrowOnConstruct>(MEMORY_USAGE_MISC), std::runtime_error);
//         // memory should recover to before allocation
//         CHECK_EQ(GetMemoryProfile(MEMORY_USAGE_MISC).Current, before);
//     }
// }

// TEST_SUITE("HeapNew / HeapDelete") {

//     struct TestObj {
//         int x;
//         TestObj(int val) : x(val) {}
//     };

//     TEST_CASE("Create and destroy") {
//         REQUIRE_ZERO(MEMORY_USAGE_MISC);
//         auto* obj = HeapNew<TestObj>(MEMORY_USAGE_MISC, 42);
//         REQUIRE(obj != nullptr);
//         CHECK_EQ(obj->x, 42);

//         HeapDelete(obj);
//         REQUIRE_ZERO(MEMORY_USAGE_MISC);
//     }
// }

// TEST_SUITE("Scope and custom deleter") {

//     struct TestObj {
//         int x;
//         TestObj(int v) : x(v) {}
//     };

//     TEST_CASE("Scope with factory function (explicit usage)") {
//         REQUIRE_ZERO(MEMORY_USAGE_RENDER);
//         {
//             auto scoped = CreateScope<TestObj>(MEMORY_USAGE_RENDER, 50);
//             REQUIRE(scoped != nullptr);
//             CHECK_EQ(scoped->x, 50);
//         }
//         REQUIRE_ZERO(MEMORY_USAGE_RENDER);
//     }

//     TEST_CASE("Scope manually reset") {
//         REQUIRE_ZERO(MEMORY_USAGE_ASSET);
//         auto scoped = CreateScope<TestObj>(MEMORY_USAGE_ASSET, 10);
//         REQUIRE(scoped);
//         // manually reset
//         scoped.reset();
//         REQUIRE_ZERO(MEMORY_USAGE_ASSET);
//     }
// }

// TEST_SUITE("StlAllocator") {

//     TEST_CASE("Vector with StlAllocator") {
//         REQUIRE_ZERO(MEMORY_USAGE_PHYSICS);
//         {
//             using VectorAlloc = StlAllocator<int, MEMORY_USAGE_PHYSICS>;
//             std::vector<int, VectorAlloc> vec;
//             vec.push_back(1);
//             vec.push_back(2);
//             vec.push_back(3);

//             // allocated amount should be >= capacity * sizeof(int)
//             CHECK_GE(GetMemoryProfile(MEMORY_USAGE_PHYSICS).Current, vec.capacity() *
//             sizeof(int));
//         }
//         // after vector destruction, all memory should be freed
//         REQUIRE_ZERO(MEMORY_USAGE_PHYSICS);
//     }
// }

// TEST_SUITE("TypeMemoryTracker") {

//     struct TestTypeA {
//         int dummy;
//     };
//     struct TestTypeB {
//         char dummy;
//     };

//     // get type IDs
//     size_t idA = GetTypeId<TestTypeA>();
//     size_t idB = GetTypeId<TestTypeB>();

//     TEST_CASE("Record allocation and deallocation") {
//         auto& tracker = TypeMemoryTracker::Get();
//         tracker.Reset(); // start from clean state

//         constexpr size_t sizeA = 64;
//         tracker.RecordAlloc(idA, sizeA);
//         MemoryProfile profA = tracker.GetTypeProfile(idA);
//         CHECK_EQ(profA.Current, sizeA);
//         CHECK_GE(profA.Peak, sizeA);

//         tracker.RecordDealloc(idA, sizeA);
//         profA = tracker.GetTypeProfile(idA);
//         CHECK_EQ(profA.Current, 0);
//         CHECK_GE(profA.Peak, sizeA); // peak remains unchanged
//     }

//     TEST_CASE("Peak tracking per type") {
//         auto& tracker = TypeMemoryTracker::Get();
//         tracker.Reset();

//         tracker.RecordAlloc(idA, 32);
//         tracker.RecordAlloc(idA, 48);
//         tracker.RecordDealloc(idA, 48);

//         MemoryProfile prof = tracker.GetTypeProfile(idA);
//         CHECK_EQ(prof.Current, 32);
//         CHECK_GE(prof.Peak, 80); // peak 32+48
//     }

//     TEST_CASE("Leak detection per type") {
//         auto& tracker = TypeMemoryTracker::Get();
//         tracker.Reset();

//         // intentionally leak 200 bytes for idB
//         tracker.RecordAlloc(idB, 200);

//         MemoryProfile leaks[16];
//         int count = tracker.GetTypeLeaks(leaks, 16);
//         REQUIRE_GE(count, 1);

//         bool found = false;
//         for (int i = 0; i < count; ++i) {
//             if (leaks[i].Current == 200) { // simple check; could extend Usage field (not used
//             here)
//                 found = true;
//                 break;
//             }
//         }
//         CHECK(found);

//         // clean up leak
//         tracker.RecordDealloc(idB, 200);
//     }

//     TEST_CASE("Reset clears all data") {
//         auto& tracker = TypeMemoryTracker::Get();
//         tracker.Reset();

//         tracker.RecordAlloc(idA, 100);
//         tracker.Reset();
//         MemoryProfile prof = tracker.GetTypeProfile(idA);
//         CHECK_EQ(prof.Current, 0);
//     }

//     TEST_CASE("Integration with New/Delete") {
//         struct Tracked {
//             int val;
//             Tracked(int v) : val(v) {}
//         };

//         auto& tracker = TypeMemoryTracker::Get();
//         tracker.Reset();

//         // New should automatically record allocation
//         auto* obj          = New<Tracked>(MEMORY_USAGE_MISC, 10);
//         size_t tid         = GetTypeId<Tracked>();
//         MemoryProfile prof = tracker.GetTypeProfile(tid);
//         CHECK_EQ(prof.Current, sizeof(Tracked));

//         // Delete should automatically record deallocation
//         Delete(obj);
//         prof = tracker.GetTypeProfile(tid);
//         CHECK_EQ(prof.Current, 0);
//     }
// }