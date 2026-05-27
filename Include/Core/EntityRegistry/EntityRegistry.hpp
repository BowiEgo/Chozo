#pragma once

#include <Core/Header/Handle.hpp>

namespace CZ {

struct DefaultStoragePolicy {
    template <typename T, typename... Args> T* Allocate(Args&&... args) {
        return new T(std::forward<Args>(args)...);
    }

    template <typename T> void Deallocate(T* ptr) { delete ptr; }

    template <typename T> T* Get(T* ptr) { return ptr; }
};

template <typename T, typename Policy = DefaultStoragePolicy> class EntityRegistry : public Policy {
public:
    virtual void Init() = 0;

    template <typename... Args> Handle<T> Create(Args&&... args) {
        T* ptr = this->Allocate(std::forward<Args>(args)...);
        return Handle<T>(ptr);
    }

    void Destroy(Handle<T>& handle) {
        if (T* ptr = handle.get()) {
            this->Deallocate(ptr);
            handle = Handle<T>();
        }
    }

    T* Get(Handle<T> handle) { return this->Policy::Get(handle.get()); }

    template <typename U = Policy>
    auto ForEach(std::invoke_result_t<decltype(&U::begin), U> = nullptr)
        -> decltype(std::declval<U>().begin()) {
        return this->begin();
    }

protected:
    using Policy::Allocate;
    using Policy::Deallocate;
};
} // namespace CZ
