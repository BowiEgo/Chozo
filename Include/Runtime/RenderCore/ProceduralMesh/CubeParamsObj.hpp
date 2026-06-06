#pragma once

#include "Core/Memory/MemoryTypes.hpp"
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct CubeParamsObj : public MeshParamsObj {
    float Width             = 1.0f;
    float Height            = 1.0f;
    float Depth             = 1.0f;
    uint32_t WidthSegments  = 1;
    uint32_t HeightSegments = 1;
    uint32_t DepthSegments  = 1;

    CubeParamsObj() = default;

    CubeParamsObj(float width, float height, float depth, uint32_t widthSegments,
                  uint32_t heightSegments, uint32_t depthSegments)
        : Width(width), Height(height), Depth(depth), WidthSegments(widthSegments),
          HeightSegments(heightSegments), DepthSegments(depthSegments) {}

    CubeParamsObj(const CubeParamsObj& other)
        : MeshParamsObj(other), Width(other.Width), Height(other.Height), Depth(other.Depth),
          WidthSegments(other.WidthSegments), HeightSegments(other.HeightSegments),
          DepthSegments(other.DepthSegments) {}

    // ===== Params Implementation =====
    virtual MeshParamsObj* Clone() const override {
        return CZ_NEW(MEMORY_USAGE_RENDER, CubeParamsObj, *this);
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
    virtual std::string GetTypeName() const override { return "Cube"; }
    static const char* GetStaticTypeName() { return "Cube"; }

    // ===== Comparison Operators =====
    bool operator==(const CubeParamsObj& other) const { return Equals_Internal(other); }
    bool operator!=(const CubeParamsObj& other) const { return !(*this == other); }
};

} // namespace CZ
