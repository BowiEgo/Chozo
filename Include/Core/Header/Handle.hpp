#pragma once

#include <Core/Memory/Memory.hpp>

namespace CZ {

/**
 * Non-owning view over an object that is managed elsewhere.
 *
 * A `Handle` never owns what it points at: it is copyable, cheap, and becomes dangling if the
 * owner is destroyed. Ownership is expressed with `Scope<T>` (`std::unique_ptr` + `Delete`),
 * held by the object that owns the resource, or by a container such as `AssetRegistry`.
 * Use `ViewOf()` to derive a view from an owner.
 */
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

    explicit operator bool() const { return m_Obj != nullptr; }

    // A single const overload: declaring both const and non-const versions makes `a == b`
    // ambiguous in C++20 (the reversed candidate `b == a` becomes viable).
    bool operator==(const Handle& other) const { return m_Obj == other.m_Obj; }
    bool operator!=(const Handle& other) const { return !(*this == other); }
    bool EqualObj(TObject* obj) { return m_Obj == obj; }

    TObject* operator->() { return m_Obj; }
    const TObject* operator->() const { return m_Obj; }

    TObject* Unwrap(AccessKey) { return m_Obj; }
    const TObject* Unwrap(AccessKey) const { return m_Obj; }

    /// Raw pointer access; the handle stays non-owning.
    TObject* Get() const { return m_Obj; }

protected:
    // Always start out null so that default-constructed handles are safe to test.
    TObject* m_Obj = nullptr;
};

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

/// Derives a non-owning view from an owning `Scope`.
template <typename TObject> Handle<TObject> ViewOf(const Scope<TObject>& owner) {
    return Handle<TObject>(owner.get());
}

template <typename TObject, typename TDeleter>
Handle<TObject> ViewOf(const std::unique_ptr<TObject, TDeleter>& owner) {
    return Handle<TObject>(owner.get());
}

/// Derives a view of a specific handle type (e.g. `Texture`) from an owning `Scope`.
template <typename TView, typename TObject> TView ViewAs(const Scope<TObject>& owner) {
    return TView(owner.get());
}

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