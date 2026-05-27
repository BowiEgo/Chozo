#pragma once

#include <Core/Memory/MemoryTypes.hpp>
#include <Core/Memory/TypeMemoryTracker.hpp>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>

namespace CZ {

void* HeapMalloc(std::size_t size, MemoryUsage usage);

void* HeapMallocDebug(std::size_t size, MemoryUsage usage, const char* file, uint32_t line);

void HeapFree(void* ptr);

char* HeapStrdup(const char* cstr, MemoryUsage usage);

const MemoryProfile& GetMemoryProfile(MemoryUsage usage);

int GetMemoryLeaks(MemoryProfile* leaks);

const char* GetMemoryUsageCstr(MemoryUsage usage);

void ReportMemoryLeaks();

#ifdef CZ_DEBUG
    #define HEAP_MALLOC(size, usage) HeapMallocDebug(size, usage, __FILE__, __LINE__)
#else
    #define HEAP_MALLOC(size, usage) HeapMalloc(size, usage)
#endif

template <typename T, typename... TArgs> T* HeapNew(MemoryUsage usage, TArgs&&... args) {
    T* ptr = (T*)HEAP_MALLOC(sizeof(T), usage);
    new (ptr) T(std::forward<TArgs>(args)...);
    return ptr;
}

template <typename T> void HeapDelete(T* ptr) {
    ptr->~T();
    HeapFree(ptr);
}

template <typename T, typename... Args> T* New(MemoryUsage usage, Args&&... args) {
    void* memory = HeapMalloc(sizeof(T), usage);
    if constexpr (!std::is_same_v<T, void>) {
        TypeMemoryTracker::Get().RecordAlloc(GetTypeId<T>(), sizeof(T));
    }
    try {
        return ::new (memory) T(std::forward<Args>(args)...);
    } catch (...) {
        if constexpr (!std::is_same_v<T, void>) {
            TypeMemoryTracker::Get().RecordDealloc(GetTypeId<T>(), sizeof(T));
        }
        HeapFree(memory);
        throw;
    }
}

template <typename T, typename... Args>
T* NewDebug(MemoryUsage usage, const char* file, uint32_t line, Args&&... args) {
    void* memory = HeapMallocDebug(sizeof(T), usage, file, line);
    if constexpr (!std::is_same_v<T, void>) {
        TypeMemoryTracker::Get().RecordAlloc(GetTypeId<T>(), sizeof(T));
    }
    try {
        return ::new (memory) T(std::forward<Args>(args)...);
    } catch (...) {
        if constexpr (!std::is_same_v<T, void>) {
            TypeMemoryTracker::Get().RecordDealloc(GetTypeId<T>(), sizeof(T));
        }
        HeapFree(memory);
        throw;
    }
}

template <typename T> void Delete(T* ptr) {
    if (ptr) {
        ptr->~T();
        if constexpr (!std::is_same_v<T, void>) {
            TypeMemoryTracker::Get().RecordDealloc(GetTypeId<T>(), sizeof(T));
        }
        HeapFree(ptr);
    }
}

template <typename T, MemoryUsage USAGE> class StlAllocator {
public:
    using value_type = T;

    template <typename U> struct rebind {
        using other = StlAllocator<U, USAGE>;
    };

    StlAllocator() = default;

    template <typename U> StlAllocator(const StlAllocator<U, USAGE>&) {}

    T* allocate(std::size_t n) { return static_cast<T*>(HEAP_MALLOC(n * sizeof(T), USAGE)); }

    void deallocate(T* p, std::size_t) { HeapFree(p); }

    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal                        = std::true_type;
};

struct DeleteDeleter {
    template <typename T> void operator()(T* ptr) const { Delete(ptr); }
};

template <typename T> using Scope = std::unique_ptr<T, DeleteDeleter>;

template <typename T, typename... Args> Scope<T> CreateScope(MemoryUsage usage, Args&&... args) {
    return Scope<T>(New<T>(usage, std::forward<Args>(args)...));
}

// template <typename T, typename... Args> Scope<T> CreateScope(Args&&... args) {
//     return CreateScope<T>(MemoryTraits<T>::Usage, std::forward<Args>(args)...);
// }

template <typename T, typename... Args>
Scope<T> CreateScopeDebug(MemoryUsage usage, const char* file, uint32_t line, Args&&... args) {
    return Scope<T>(NewDebug<T>(usage, file, line, std::forward<Args>(args)...));
}

#ifdef CZ_DEBUG
    #define CZ_NEW(usage, type, ...) ::CZ::NewDebug<type>(usage, __FILE__, __LINE__, ##__VA_ARGS__)
    #define CZ_CREATE_SCOPE(usage, type, ...)                                                      \
        ::CZ::CreateScopeDebug<type>(usage, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define CZ_NEW(usage, type, ...)          ::CZ::New<type>(usage, ##__VA_ARGS__)
    #define CZ_CREATE_SCOPE(usage, type, ...) ::CZ::CreateScope<type>(usage, ##__VA_ARGS__)
#endif

} // namespace CZ