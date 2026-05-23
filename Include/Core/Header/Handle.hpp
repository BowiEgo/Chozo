#pragma once

#include <Core/Memory/Memory.hpp>

namespace CZ {

template <typename TObject> class Handle {
public:
    class AccessKey {
        friend struct PoolAllocator;
        friend struct LinearAllocator;
        friend class InternalHandleReader;

        AccessKey() = default;
    };

    Handle() = default;
    Handle(TObject* obj) : m_Obj(obj) {}
    Handle(const TObject* obj) : m_Obj(obj) {}

    explicit operator bool() const { return m_Obj != nullptr; }

    bool operator==(const Handle& other) { return m_Obj == other.Unwrap(); }

    TObject* operator->() { return m_Obj; }
    const TObject* operator->() const { return m_Obj; }

    TObject* Unwrap(AccessKey) { return m_Obj; }
    const TObject* Unwrap(AccessKey) const { return m_Obj; }

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

class InternalHandleReader {
public:
    template <typename T> static T* Unwrap(Handle<T>& handle) {
        typename Handle<T>::AccessKey key;
        return handle.Unwrap(key);
    }

    template <typename T> static const T* Unwrap(const Handle<T>& handle) {
        typename Handle<T>::AccessKey key;
        return handle.Unwrap(key);
    }
};

template <typename HandleType> struct HandleHash {
    size_t operator()(const HandleType& h) const {
        return std::hash<const void*>{}(InternalHandleReader::Unwrap(h));
    }
};

template <typename HandleType> struct HandleEqual {
    bool operator()(const HandleType& a, const HandleType& b) const {
        return InternalHandleReader::Unwrap(a) == InternalHandleReader::Unwrap(b);
    }
};

template <typename HandleType>
using HandleMap =
    std::unordered_map<HandleType, uint64_t, HandleHash<HandleType>, HandleEqual<HandleType>>;

} // namespace CZ