#pragma once

#include "MathUtils.h"
#include "MeshParams.h"

struct RENDER_CORE_API FSphereParams : public IMeshParams {
    float Radius            = 0.5f;
    uint32_t WidthSegments  = 32;
    uint32_t HeightSegments = 16;
    float PhiStart          = 0.0f;
    float PhiLength         = ChozoUtils::Math::PI * 2;
    float ThetaStart        = 0.0f;
    float ThetaLength       = ChozoUtils::Math::PI;

    FSphereParams() = default;

    FSphereParams(float radius, uint32_t widthSegments, uint32_t heightSegments, float phiStart = 0,
                  float phiLength = ChozoUtils::Math::PI * 2, float thetaStart = 0,
                  float thetaLength = ChozoUtils::Math::PI)
        : Radius(radius), WidthSegments(widthSegments), HeightSegments(heightSegments),
          PhiStart(phiStart), PhiLength(phiLength), ThetaStart(thetaStart),
          ThetaLength(thetaLength) {}

    FSphereParams(const FSphereParams& other)
        : IMeshParams(other), Radius(other.Radius), WidthSegments(other.WidthSegments),
          HeightSegments(other.HeightSegments), PhiStart(other.PhiStart),
          PhiLength(other.PhiLength), ThetaStart(other.ThetaStart), ThetaLength(other.ThetaLength) {
    }

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FSphereParams(*this); }

    virtual size_t GetHash() const override;

    virtual std::any GetParamValue(const std::string& name) const override;

    // ===== IMeshParams Implementation =====
    virtual bool Equals_Internal(const IParams& other) const override;

    virtual void Accept_Internal(IParamsVisitor& visitor) override;

    virtual void Accept_Internal(IConstParamsVisitor& visitor) const override;

    virtual const std::vector<std::string>& GetAllParamNames_Internal() override;

    // ===== Type Info =====
    virtual std::string GetTypeName() const override { return "Sphere"; }
    static const char* GetStaticTypeName() { return "Sphere"; }

    // ===== Comparison Operators =====
    bool operator==(const FSphereParams& other) const { return Equals(other); }
    bool operator!=(const FSphereParams& other) const { return !(*this == other); }
};
