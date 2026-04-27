#pragma once

#include "CoreMinimal.h"
#include "MeshParams.h"
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
 * FMeshProps - Type-erased wrapper for mesh parameters with value semantics
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
 * FMeshProps sphere(FSphereParams(1.0f, 32, 16));
 * FMeshProps sphere2 = sphere;                    // Shallow copy
 * if (auto* p = sphere.As<FSphereParams>()) {
 *     p->Radius = 2.0f;                            // Modifies both sphere and sphere2
 * }
 * FMeshProps sphere3 = sphere.Clone();            // Deep copy
 * @endcode
 */
class FMeshProps {
public:
    FMeshProps() = default;

    template <typename T> explicit FMeshProps(const T& params) : m_Params(CreateRef<T>(params)) {}

    explicit FMeshProps(IMeshParams* params) : m_Params(params) {}

    explicit FMeshProps(TScope<IMeshParams> params) : m_Params(params.release()) {}

    explicit FMeshProps(TRef<IMeshParams> params) : m_Params(std::move(params)) {}

    FMeshProps(const FMeshProps& other)            = default;
    FMeshProps& operator=(const FMeshProps& other) = default;

    FMeshProps(FMeshProps&& other) noexcept            = default;
    FMeshProps& operator=(FMeshProps&& other) noexcept = default;

    // ===== Type Info =====
    std::string GetTypeName() const { return m_Params ? m_Params->GetTypeName() : ""; }
    static const char* GetStaticTypeName() { return "FMeshProps"; }

    // ===== Comparison =====
    bool operator==(const FMeshProps& other) const {
        if (!m_Params && !other.m_Params) return true;
        if (!m_Params || !other.m_Params) return false;
        return m_Params->Equals(*other.m_Params);
    }

    bool operator!=(const FMeshProps& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return m_Params ? m_Params->GetHash() : 0; }

    // ===== Validity =====
    explicit operator bool() const { return m_Params != nullptr; }

    // ===== Access =====
    IMeshParams* Get() { return m_Params.get(); }
    const IMeshParams* Get() const { return m_Params.get(); }

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
    FMeshProps Clone() const {
        if (!m_Params) return FMeshProps();
        return FMeshProps(TScope<IMeshParams>(static_cast<IMeshParams*>(m_Params->Clone())));
    }

private:
    TRef<IMeshParams> m_Params;
};
