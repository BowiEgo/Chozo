#pragma once

#include <atomic>

#include "CoreExport.h"
#include "CoreMinimal.h"

class FRefCounted {
public:
    virtual ~FRefCounted() = default;

    void IncRefCount() const { ++m_RefCount; }
    void DecRefCount() const { --m_RefCount; }

    uint32_t GetRefCount() const { return m_RefCount.load(); }

private:
    mutable std::atomic<uint32_t> m_RefCount = 0;
};

namespace RefUtils {
CORE_API void AddToLiveReferences(void* instance);
CORE_API void RemoveFromLiveReferences(void* instance);
CORE_API bool IsLive(const void* instance);
} // namespace RefUtils

template <typename T> class TRef {
public:
    TRef() : m_Instance(nullptr) {}

    TRef(std::nullptr_t n) // NOLINT
        : m_Instance(nullptr) {}

    TRef(T* instance) // NOLINT
        : m_Instance(instance) {
        static_assert(std::is_base_of_v<FRefCounted, T>,
                      "Class is not TRefCounted!");

        IncRef();
    }

    template <typename T2>
    TRef(const TRef<T2>& other) // NOLINT
    {
        m_Instance = (T*)other.m_Instance;
        IncRef();
    }

    template <typename T2>
    TRef(TRef<T2>&& other) // NOLINT
    {
        m_Instance = (T*)other.m_Instance;
        other.m_Instance = nullptr;
    }

    // Takes a raw pointer that ALREADY has a +1 ref count and wraps it
    static TRef<T> Adopt(T* instance) // NOLINT
    {
        // We call a private constructor that doesn't trigger IncRef
        return TRef<T>(instance, false);
    }

    ~TRef() { DecRef(); }

    TRef(const TRef<T>& other) : m_Instance(other.m_Instance) { IncRef(); }

    TRef& operator=(std::nullptr_t) {
        DecRef();
        m_Instance = nullptr;
        return *this;
    }

    TRef& operator=(const TRef<T>& other) {
        if (this == &other)
            return *this;

        other.IncRef();
        DecRef();

        m_Instance = other.m_Instance;
        return *this;
    }

    template <typename T2> TRef& operator=(const TRef<T2>& other) {
        other.IncRef();
        DecRef();

        m_Instance = other.m_Instance;
        return *this;
    }

    template <typename T2> TRef& operator=(TRef<T2>&& other) {
        DecRef();

        m_Instance = other.m_Instance;
        other.m_Instance = nullptr;
        return *this;
    }

    explicit operator bool() { return m_Instance != nullptr; }
    explicit operator bool() const { return m_Instance != nullptr; }

    T* operator->() { return m_Instance; }
    const T* operator->() const { return m_Instance; }

    T& operator*() { return *m_Instance; }
    const T& operator*() const { return *m_Instance; }

    T* Raw() { return m_Instance; }
    const T* Raw() const { return m_Instance; }

    T* get() { return m_Instance; }
    const T* get() const { return m_Instance; }

    void Reset(T* instance = nullptr) {
        DecRef();
        m_Instance = instance;
    }

    template <typename T2> TRef<T2> As() const { return TRef<T2>(*this); }

    template <typename... Args> static TRef<T> Create(Args&&... args) {
#if defined(ENGINE_PLATFORM_WIN32)
        return TRef<T>(new (typeid(T).name()) T(std::forward<Args>(args)...));
#else
        return TRef<T>(new T(std::forward<Args>(args)...));
#endif
    }

    bool operator==(const TRef<T>& other) const {
        return m_Instance == other.m_Instance;
    }

    bool operator!=(const TRef<T>& other) const { return !(*this == other); }

    bool EqualsObject(const TRef<T>& other) {
        if (!m_Instance || !other.m_Instance)
            return false;

        return *m_Instance == *other.m_Instance;
    }

private:
    TRef(T* instance, bool increment) : m_Instance(instance) {
        if (m_Instance && increment) {
            IncRef();
        }
    }

    void IncRef() const {
        if (m_Instance) {
            m_Instance->IncRefCount();
            RefUtils::AddToLiveReferences((void*)m_Instance);
        }
    }

    void DecRef() const {
        if (m_Instance) {
            m_Instance->DecRefCount();

            if (m_Instance->GetRefCount() == 0) {
                delete m_Instance;
                RefUtils::RemoveFromLiveReferences((void*)m_Instance);
                m_Instance = nullptr;
            }
        }
    }

    template <class T2> friend class TRef;
    mutable T* m_Instance;
};

template <typename T, typename... Args>
constexpr TRef<T> CreateRef(Args&&... args) {
    return TRef<T>::Create(std::forward<Args>(args)...);
}

template <typename T> class WeakRef {
public:
    WeakRef() = default;

    WeakRef(TRef<T> ref) // NOLINT
    {
        m_Instance = ref.Raw();
    }

    WeakRef(T* instance) // NOLINT
    {
        m_Instance = instance;
    }

    T* operator->() { return m_Instance; }
    const T* operator->() const { return m_Instance; }

    T& operator*() { return *m_Instance; }
    const T& operator*() const { return *m_Instance; }

    [[nodiscard]] bool IsValid() const {
        return m_Instance ? RefUtils::IsLive(m_Instance) : false;
    }
    explicit operator bool() const { return IsValid(); }

    template <typename T2> WeakRef<T2> As() const {
        return WeakRef<T2>(dynamic_cast<T2*>(m_Instance));
    }

private:
    T* m_Instance = nullptr;
};
