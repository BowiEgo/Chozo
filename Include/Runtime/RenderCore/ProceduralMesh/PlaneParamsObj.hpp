#pragma once
#include <any>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "Core/Memory/MemoryTypes.hpp"
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct PlaneParamsObj : public MeshParamsObj {
    float Width             = 1.0f;
    float Height            = 1.0f;
    uint32_t WidthSegments  = 1;
    uint32_t HeightSegments = 1;

    PlaneParamsObj() = default;

    PlaneParamsObj(float width, float height, uint32_t widthSegments, uint32_t heightSegments)
        : Width(width), Height(height), WidthSegments(widthSegments),
          HeightSegments(heightSegments) {}

    PlaneParamsObj(const PlaneParamsObj& other)
        : MeshParamsObj(other), Width(other.Width), Height(other.Height),
          WidthSegments(other.WidthSegments), HeightSegments(other.HeightSegments) {}

    // ===== Params Implementation =====
    virtual MeshParamsObj* Clone() const override {
        return CZ_NEW(MEMORY_USAGE_RENDER, PlaneParamsObj, *this);
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
    virtual std::string GetTypeName() const override { return "Plane"; }
    static const char* GetStaticTypeName() { return "Plane"; }

    // ===== Comparison Operators =====
    bool operator==(const PlaneParamsObj& other) const { return Equals_Internal(other); }
    bool operator!=(const PlaneParamsObj& other) const { return !(*this == other); }
};

} // namespace CZ
