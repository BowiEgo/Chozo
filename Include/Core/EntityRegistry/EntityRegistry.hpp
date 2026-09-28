#pragma once
#include <utility>

#include <Core/Header/Handle.hpp>

namespace CZ {

/**
 * Policies decide how a registry allocates and (for container-backed policies) owns objects.
 * `EntityRegistry` deliberately exposes no `Destroy`: freeing belongs to whoever owns the object
 * (the policy's container, or the caller when it holds a `Scope`).
 */
struct DefaultStoragePolicy {
    template <typename T, typename... Args> T* Allocate(Args&&... args) {
        return new T(std::forward<Args>(args)...);
    }

    template <typename T> void Deallocate(T* ptr) { delete ptr; }

    template <typename T> T* Get(T* ptr) { return ptr; }
};

template <typename T, typename Policy = DefaultStoragePolicy> class EntityRegistry : public Policy {
public:
    virtual ~EntityRegistry() = default;

    virtual void Init()     = 0;
    virtual void Shutdown() = 0;

    template <typename... Args> Handle<T> Create(Args&&... args) {
        T* ptr = this->template Allocate<T>(std::forward<Args>(args)...);
        return Handle<T>(ptr);
    }

    T* Get(Handle<T> handle) { return Policy::Get(InternalHandleReader::Unwrap(handle)); }

    /// Iteration accessors, provided by policies that keep a container of objects.
    auto Begin() { return this->begin(); }
    auto End() { return this->end(); }

protected:
    using Policy::Allocate;
};
} // namespace CZ
