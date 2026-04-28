#pragma once

#include "CoreMinimal.h"
#include "MaterialParams.h"
#include "Params.h"
#include "Ref.h"
#include "Scope.h"

#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <functional>
#include <variant>

class RENDER_CORE_API FMaterialParamsWrapper {
public:
    FMaterialParamsWrapper() = default;

    template <typename T>
    explicit FMaterialParamsWrapper(const T& params) : m_Params(CreateRef<T>(params)) {}

    explicit FMaterialParamsWrapper(IMaterialParams* params) : m_Params(params) {}

    explicit FMaterialParamsWrapper(TScope<IMaterialParams> params) : m_Params(params.release()) {}

    explicit FMaterialParamsWrapper(TRef<IMaterialParams> params) : m_Params(std::move(params)) {}

    FMaterialParamsWrapper(const FMaterialParamsWrapper& other)            = default;
    FMaterialParamsWrapper& operator=(const FMaterialParamsWrapper& other) = default;

    FMaterialParamsWrapper(FMaterialParamsWrapper&& other) noexcept            = default;
    FMaterialParamsWrapper& operator=(FMaterialParamsWrapper&& other) noexcept = default;

    // ===== Type Info =====
    std::string GetTypeName() const { return m_Params ? m_Params->GetTypeName() : ""; }
    static const char* GetStaticTypeName() { return "FMaterialParamsWrapper"; }

    // ===== Comparison =====
    bool operator==(const FMaterialParamsWrapper& other) const {
        if (!m_Params && !other.m_Params) return true;
        if (!m_Params || !other.m_Params) return false;
        return m_Params->Equals(*other.m_Params);
    }

    bool operator!=(const FMaterialParamsWrapper& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return m_Params ? m_Params->GetHash() : 0; }

    // ===== Validity =====
    explicit operator bool() const { return m_Params != nullptr; }

    // ===== Access =====
    IMaterialParams* Get() { return m_Params.get(); }
    const IMaterialParams* Get() const { return m_Params.get(); }

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
    FMaterialParamsWrapper Clone() const {
        if (!m_Params) return FMaterialParamsWrapper();
        return FMaterialParamsWrapper(
            TScope<IMaterialParams>(static_cast<IMaterialParams*>(m_Params->Clone())));
    }

private:
    TRef<IMaterialParams> m_Params;
};
