#pragma once

#include "MeshParams.h"
#include "MeshTypes.h"

struct RENDER_CORE_API FCubeParams : public IMeshParams {
    float Width             = 1.0f;
    float Height            = 1.0f;
    float Depth             = 1.0f;
    uint32_t WidthSegments  = 1;
    uint32_t HeightSegments = 1;
    uint32_t DepthSegments  = 1;

    FCubeParams() = default;

    FCubeParams(float width, float height, float depth, uint32_t widthSegments,
                uint32_t heightSegments, uint32_t depthSegments)
        : Width(width), Height(height), Depth(depth), WidthSegments(widthSegments),
          HeightSegments(heightSegments), DepthSegments(depthSegments) {}

    FCubeParams(const FCubeParams& other)
        : IMeshParams(other), Width(other.Width), Height(other.Height), Depth(other.Depth),
          WidthSegments(other.WidthSegments), HeightSegments(other.HeightSegments),
          DepthSegments(other.DepthSegments) {}

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FCubeParams(*this); }

    virtual size_t GetHash() const override;

    virtual std::any GetParamValue(const std::string& name) const override;

    // ===== IMeshParams Implementation =====
    virtual bool Equals_Internal(const IParams& other) const override;

    virtual void Accept_Internal(IParamsVisitor& visitor) override;

    virtual void Accept_Internal(IConstParamsVisitor& visitor) const override;

    virtual const std::vector<std::string>& GetAllParamNames_Internal() override;

    // ===== Type Info =====
    virtual std::string GetTypeName() const override { return "Cube"; }
    static const char* GetStaticTypeName() { return "Cube"; }

    // ===== Comparison Operators =====
    bool operator==(const FCubeParams& other) const { return Equals(other); }
    bool operator!=(const FCubeParams& other) const { return !(*this == other); }
};
