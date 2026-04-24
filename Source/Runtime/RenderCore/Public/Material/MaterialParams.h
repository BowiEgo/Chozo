#pragma once

#include "CoreMinimal.h"
#include "Params.h"
#include "Ref.h"
#include "Scope.h"

#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <functional>
#include <variant>

class FMaterialParams {
public:
    FMaterialParams() = default;

    template <typename T>
    explicit FMaterialParams(const T& params) : m_Params(CreateRef<T>(params)) {}

    explicit FMaterialParams(IParams* params) : m_Params(params) {}

    explicit FMaterialParams(TScope<IParams> params) : m_Params(params.release()) {}

    explicit FMaterialParams(TRef<IParams> params) : m_Params(std::move(params)) {}

    FMaterialParams(const FMaterialParams& other)            = default;
    FMaterialParams& operator=(const FMaterialParams& other) = default;

    FMaterialParams(FMaterialParams&& other) noexcept            = default;
    FMaterialParams& operator=(FMaterialParams&& other) noexcept = default;

    // ===== Type Info =====
    std::string GetTypeName() const { return m_Params ? m_Params->GetTypeName() : ""; }
    static const char* GetStaticTypeName() { return "FMaterialParams"; }

    // ===== Comparison =====
    bool operator==(const FMaterialParams& other) const {
        if (!m_Params && !other.m_Params) return true;
        if (!m_Params || !other.m_Params) return false;
        return m_Params->Equals(*other.m_Params);
    }

    bool operator!=(const FMaterialParams& other) const { return !(*this == other); }

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
    FMaterialParams Clone() const {
        if (!m_Params) return FMaterialParams();
        return FMaterialParams(m_Params->Clone());
    }

private:
    TRef<IParams> m_Params;
};
