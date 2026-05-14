#pragma once

#include <Core/Memory/Memory.h>

namespace CZ {

template <typename TObject> class Handle {
public:
    class AccessKey {
        friend struct PoolAllocator;
        friend struct LinearAllocator;
        friend class RHIInternalReader;
        friend class WindowInternalReader;

        AccessKey() = default;
    };

    Handle() = default;
    Handle(TObject* obj) : m_Obj(obj) {}
    Handle(const TObject* obj) : m_Obj(obj) {}

    explicit operator bool() const { return m_Obj != nullptr; }

    bool operator==(const Handle& other) { return m_Obj == other.Unwrap(); }

    TObject* Unwrap(AccessKey) { return m_Obj; }
    void Destroy();

protected:
    TObject* m_Obj;
};

#define DEFINE_HANDLE_DESTROY(T)                                                                   \
    template <> void Handle<T>::Destroy() {                                                        \
        if (m_Obj) {                                                                               \
            Delete(m_Obj);                                                                         \
            m_Obj = nullptr;                                                                       \
        }                                                                                          \
    }

class RHIInternalReader {
public:
    template <typename T> static T* Unwrap(Handle<T>& handle) {
        typename Handle<T>::AccessKey key;
        return handle.Unwrap(key);
    }
};

class WindowInternalReader {
public:
    template <typename T> static T* Unwrap(Handle<T>& handle) {
        typename Handle<T>::AccessKey key;
        return handle.Unwrap(key);
    }
};

} // namespace CZ