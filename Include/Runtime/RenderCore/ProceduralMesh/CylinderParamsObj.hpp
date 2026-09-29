#pragma once
#include <any>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "Core/Memory/MemoryTypes.hpp"
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct CylinderParamsObj : public MeshParamsObj {
    float Radius            = 1.0f;
    float Height            = 1.0f;
    uint32_t RadialSegments = 32;
    uint32_t HeightSegments = 1;

    CylinderParamsObj() = default;

    CylinderParamsObj(float radius, float height, uint32_t radialSegments, uint32_t heightSegments)
        : Radius(radius), Height(height), RadialSegments(radialSegments),
          HeightSegments(heightSegments) {}

    CylinderParamsObj(const CylinderParamsObj& other)
        : MeshParamsObj(other), Radius(other.Radius), Height(other.Height),
          RadialSegments(other.RadialSegments), HeightSegments(other.HeightSegments) {}

    // ===== Params Implementation =====
    virtual MeshParamsObj* Clone() const override {
        return CZ_NEW(MEMORY_USAGE_RENDER, CylinderParamsObj, *this);
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
    virtual std::string GetTypeName() const override { return "Cylinder"; }
    static const char* GetStaticTypeName() { return "Cylinder"; }

    // ===== Comparison Operators =====
    bool operator==(const CylinderParamsObj& other) const { return Equals_Internal(other); }
    bool operator!=(const CylinderParamsObj& other) const { return !(*this == other); }
};

} // namespace CZ
