#pragma once

#include <Core/Memory/MemoryTypes.h>
#include <Core/Memory/TypeMemoryTracker.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

namespace CZ {

void* HeapMalloc(std::size_t size, MemoryUsage usage);

void HeapFree(void* ptr);

char* HeapStrdup(const char* cstr, MemoryUsage usage);

const MemoryProfile& GetMemoryProfile(MemoryUsage usage);

int GetMemoryLeaks(MemoryProfile* leaks);

const char* GetMemoryUsageCstr(MemoryUsage usage);

template <typename T, typename... TArgs> T* HeapNew(MemoryUsage usage, TArgs&&... args) {
    T* ptr = (T*)HeapMalloc(sizeof(T), usage);
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

template <typename T> struct MemoryTraits {
    static constexpr MemoryUsage Usage = MEMORY_USAGE_MISC;
};

template <typename T, typename... Args> T* New(Args&&... args) {
    return New<T>(MemoryTraits<T>::Usage, std::forward<Args>(args)...);
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

    T* allocate(std::size_t n) { return static_cast<T*>(HeapMalloc(n * sizeof(T), USAGE)); }

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

template <typename T, typename... Args> Scope<T> CreateScope(Args&&... args) {
    return CreateScope<T>(MemoryTraits<T>::Usage, std::forward<Args>(args)...);
}

} // namespace CZ