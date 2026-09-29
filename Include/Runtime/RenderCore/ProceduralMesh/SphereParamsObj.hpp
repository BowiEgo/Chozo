#pragma once
#include <any>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "Core/Memory/MemoryTypes.hpp"
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct SphereParamsObj : public MeshParamsObj {
    float Radius            = 1.0f;
    uint32_t WidthSegments  = 32;
    uint32_t HeightSegments = 16;
    float PhiStart          = 0.0f;
    float PhiLength         = 6.283185307179586f; // 2 * PI
    float ThetaStart        = 0.0f;
    float ThetaLength       = 3.141592653589793f; // PI

    SphereParamsObj() = default;

    SphereParamsObj(float radius, uint32_t widthSegments, uint32_t heightSegments)
        : Radius(radius), WidthSegments(widthSegments), HeightSegments(heightSegments) {}

    SphereParamsObj(const SphereParamsObj& other)
        : MeshParamsObj(other), Radius(other.Radius), WidthSegments(other.WidthSegments),
          HeightSegments(other.HeightSegments), PhiStart(other.PhiStart),
          PhiLength(other.PhiLength), ThetaStart(other.ThetaStart), ThetaLength(other.ThetaLength) {
    }

    // ===== Params Implementation =====
    virtual MeshParamsObj* Clone() const override {
        return CZ_NEW(MEMORY_USAGE_RENDER, SphereParamsObj, *this);
    }

    virtual size_t GetHash() const override;

    virtual std::any GetParamValue(const std::string& name) const override;

    // ===== MeshParams Implementation =====
    virtual bool Equals_Internal(MeshParamsObj& other) override;

    virtual bool Equals_Internal(const MeshParamsObj& other) const override;

    virtual void Accept_Internal(ParamsVisitor& visitor) override;

    virtual void Accept_Internal(ConstParamsVisitor& visitor) const override;

    virtual const std::vector<std::string>& GetAllParamNames_Internal() override;

    // ===== Type Info =====
    virtual std::string GetTypeName() const override { return "Sphere"; }
    static const char* GetStaticTypeName() { return "Sphere"; }

    // ===== Comparison Operators =====
    bool operator==(const SphereParamsObj& other) const { return Equals_Internal(other); }
    bool operator!=(const SphereParamsObj& other) const { return !(*this == other); }
};

} // namespace CZ
