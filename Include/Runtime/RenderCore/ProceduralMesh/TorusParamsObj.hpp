#pragma once
#include <any>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "Core/Memory/MemoryTypes.hpp"
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct TorusParamsObj : public MeshParamsObj {
    float Radius             = 1.0f;
    float Tube               = 0.35f;
    uint32_t RadialSegments  = 32;
    uint32_t TubularSegments = 16;

    TorusParamsObj() = default;

    TorusParamsObj(float radius, float tube, uint32_t radialSegments, uint32_t tubularSegments)
        : Radius(radius), Tube(tube), RadialSegments(radialSegments),
          TubularSegments(tubularSegments) {}

    TorusParamsObj(const TorusParamsObj& other)
        : MeshParamsObj(other), Radius(other.Radius), Tube(other.Tube),
          RadialSegments(other.RadialSegments), TubularSegments(other.TubularSegments) {}

    // ===== Params Implementation =====
    virtual MeshParamsObj* Clone() const override {
        return CZ_NEW(MEMORY_USAGE_RENDER, TorusParamsObj, *this);
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
    virtual std::string GetTypeName() const override { return "Torus"; }
    static const char* GetStaticTypeName() { return "Torus"; }

    // ===== Comparison Operators =====
    bool operator==(const TorusParamsObj& other) const { return Equals_Internal(other); }
    bool operator!=(const TorusParamsObj& other) const { return !(*this == other); }
};

} // namespace CZ
