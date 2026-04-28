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
 * FMeshParamsWrapper - Type-erased wrapper for mesh parameters with value semantics
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
 * FMeshParamsWrapper sphere(FSphereParams(1.0f, 32, 16));
 * FMeshParamsWrapper sphere2 = sphere;                    // Shallow copy
 * if (auto* p = sphere.As<FSphereParams>()) {
 *     p->Radius = 2.0f;                            // Modifies both sphere and sphere2
 * }
 * FMeshParamsWrapper sphere3 = sphere.Clone();            // Deep copy
 * @endcode
 */
class RENDER_CORE_API FMeshParamsWrapper {
public:
    FMeshParamsWrapper() = default;

    template <typename T>
    explicit FMeshParamsWrapper(const T& params) : m_Params(CreateRef<T>(params)) {}

    explicit FMeshParamsWrapper(IMeshParams* params) : m_Params(params) {}

    explicit FMeshParamsWrapper(TScope<IMeshParams> params) : m_Params(params.release()) {}

    explicit FMeshParamsWrapper(TRef<IMeshParams> params) : m_Params(std::move(params)) {}

    FMeshParamsWrapper(const FMeshParamsWrapper& other)            = default;
    FMeshParamsWrapper& operator=(const FMeshParamsWrapper& other) = default;

    FMeshParamsWrapper(FMeshParamsWrapper&& other) noexcept            = default;
    FMeshParamsWrapper& operator=(FMeshParamsWrapper&& other) noexcept = default;

    // ===== Type Info =====
    std::string GetTypeName() const { return m_Params ? m_Params->GetTypeName() : ""; }
    static const char* GetStaticTypeName() { return "FMeshParamsWrapper"; }

    // ===== Comparison =====
    bool operator==(const FMeshParamsWrapper& other) const {
        if (!m_Params && !other.m_Params) return true;
        if (!m_Params || !other.m_Params) return false;
        return m_Params->Equals(*other.m_Params);
    }

    bool operator!=(const FMeshParamsWrapper& other) const { return !(*this == other); }

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
    FMeshParamsWrapper Clone() const {
        if (!m_Params) return FMeshParamsWrapper();
        return FMeshParamsWrapper(
            TScope<IMeshParams>(static_cast<IMeshParams*>(m_Params->Clone())));
    }

private:
    TRef<IMeshParams> m_Params;
};
