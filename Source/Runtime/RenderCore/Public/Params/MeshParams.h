#pragma once

#include "CoreMinimal.h"
#include "Params.h"
#include "Ref.h"
#include "Scope.h"

#include "Matrix4.h"
#include "MeshTypes.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <functional>
#include <variant>

/**
 * FMeshParams - Type-erased wrapper for mesh parameters with value semantics
 *
 * Features:
 * - Shallow copy (shared IParams) via default copy operations
 * - Deep copy via Clone() method
 * - Type-safe access via As<T>()
 * - Automatic reference counting via TRef
 * - Equality comparison based on parameter content
 *
 * Example:
 * @code
 * FMeshParams sphere(FSphereParams(1.0f, 32, 16));
 * FMeshParams sphere2 = sphere;                    // Shallow copy
 * if (auto* p = sphere.As<FSphereParams>()) {
 *     p->Radius = 2.0f;                            // Modifies both sphere and sphere2
 * }
 * FMeshParams sphere3 = sphere.Clone();            // Deep copy
 * @endcode
 */
class FMeshParams {
public:
    FMeshParams() = default;

    template <typename T> explicit FMeshParams(const T& params) : m_Params(CreateRef<T>(params)) {}

    explicit FMeshParams(IParams* params) : m_Params(params) {}

    explicit FMeshParams(TScope<IParams> params) : m_Params(params.release()) {}

    explicit FMeshParams(TRef<IParams> params) : m_Params(std::move(params)) {}

    FMeshParams(const FMeshParams& other) = default;
    FMeshParams& operator=(const FMeshParams& other) = default;

    FMeshParams(FMeshParams&& other) noexcept = default;
    FMeshParams& operator=(FMeshParams&& other) noexcept = default;

    // ===== Type Info =====
    std::string GetTypeName() const { return m_Params ? m_Params->GetTypeName() : ""; }
    static const char* GetStaticTypeName() { return "FMeshParams"; }

    // ===== Comparison =====
    bool operator==(const FMeshParams& other) const {
        if (!m_Params && !other.m_Params) return true;
        if (!m_Params || !other.m_Params) return false;
        return m_Params->Equals(*other.m_Params);
    }

    bool operator!=(const FMeshParams& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return m_Params ? m_Params->GetHash() : 0; }

    // ===== Validity =====
    explicit operator bool() const { return m_Params != nullptr; }

    // ===== Access =====
    IParams* Get() { return m_Params.get(); }
    const IParams* Get() const { return m_Params.get(); }

    template <typename T> T* As() {
        if (!m_Params) return nullptr;
        if (m_Params->GetTypeName() != T::GetStaticTypeName()) return nullptr;
        return static_cast<T*>(m_Params.get());
    }

    template <typename T> const T* As() const {
        if (!m_Params) return nullptr;
        if (m_Params->GetTypeName() != T::GetStaticTypeName()) return nullptr;
        return static_cast<const T*>(m_Params.get());
    }

    // ===== Clone =====
    FMeshParams Clone() const {
        if (!m_Params) return FMeshParams();
        return FMeshParams(m_Params->Clone());
    }

private:
    TRef<IParams> m_Params;
};

class FDynamicVariant {
public:
    template <typename T> void RegisterType(const std::string& typeName) {
        m_TypeNames.push_back(typeName);
        m_TypeIndex[typeName] = m_TypeNames.size() - 1;
    }

    template <typename T> void Set(const T& value) {
        m_Data = CreateScope<T>(value);
        m_TypeName = T::GetStaticTypeName();
    }

    template <typename T> T* Get() {
        if (m_TypeName == T::GetStaticTypeName()) {
            return static_cast<T*>(m_Data.get());
        }
        return nullptr;
    }

    const std::string& GetTypeName() const { return m_TypeName; }

private:
    TScope<void> m_Data;
    std::string m_TypeName;
    std::vector<std::string> m_TypeNames;
    std::unordered_map<std::string, size_t> m_TypeIndex;
};
