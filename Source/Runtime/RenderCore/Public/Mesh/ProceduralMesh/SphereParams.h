#pragma once

#include "Asset.h"
#include "MathUtils.h"
#include "Params.h"

struct FSphereParams : public IParams {
    float Radius            = 0.5f;
    uint32_t WidthSegments  = 32;
    uint32_t HeightSegments = 16;
    float PhiStart          = 0.0f;
    float PhiLength         = ChozoUtils::Math::PI * 2;
    float ThetaStart        = 0.0f;
    float ThetaLength       = ChozoUtils::Math::PI;

    FAssetHandle Material;

    FSphereParams() = default;

    FSphereParams(float radius, uint32_t widthSegments, uint32_t heightSegments, float phiStart = 0,
                  float phiLength = ChozoUtils::Math::PI * 2, float thetaStart = 0,
                  float thetaLength = ChozoUtils::Math::PI)
        : Radius(radius), WidthSegments(widthSegments), HeightSegments(heightSegments),
          PhiStart(phiStart), PhiLength(phiLength), ThetaStart(thetaStart),
          ThetaLength(thetaLength) {}

    FSphereParams(const FSphereParams& other)
        : Radius(other.Radius), WidthSegments(other.WidthSegments),
          HeightSegments(other.HeightSegments), PhiStart(other.PhiStart),
          PhiLength(other.PhiLength), ThetaStart(other.ThetaStart), ThetaLength(other.ThetaLength) {
    }

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FSphereParams(*this); }

    virtual bool Equals(const IParams& other) const override {
        // Type check
        const auto* otherSphere = dynamic_cast<const FSphereParams*>(&other);
        if (!otherSphere) return false;

        return Radius == otherSphere->Radius && WidthSegments == otherSphere->WidthSegments &&
               HeightSegments == otherSphere->HeightSegments && PhiStart == otherSphere->PhiStart &&
               PhiLength == otherSphere->PhiLength && ThetaStart == otherSphere->ThetaStart &&
               ThetaLength == otherSphere->ThetaLength;
    }

    virtual size_t GetHash() const override {
        size_t h = 0;
        HashCombine(h, std::hash<float>{}(Radius));
        HashCombine(h, std::hash<uint32_t>{}(WidthSegments));
        HashCombine(h, std::hash<uint32_t>{}(HeightSegments));
        HashCombine(h, std::hash<float>{}(PhiStart));
        HashCombine(h, std::hash<float>{}(PhiLength));
        HashCombine(h, std::hash<float>{}(ThetaStart));
        HashCombine(h, std::hash<float>{}(ThetaLength));
        HashCombine(h, std::hash<FAssetHandle>{}(Material));
        return h;
    }

    virtual std::string GetTypeName() const override { return "Sphere"; }

    virtual size_t GetPropertyCount() const override { return 8; }
    virtual std::string GetPropertyName(size_t index) const override {
        static const std::string names[] = { "Radius",    "Segments",   "Rings",       "PhiStart",
                                             "PhiLength", "ThetaStart", "ThetaLength", "Material" };
        return index < GetPropertyCount() ? names[index] : "";
    }

    virtual void Accept(IParamsVisitor& visitor) override {
        visitor.Visit(Radius, "Radius");
        visitor.Visit(WidthSegments, "Width Segments");
        visitor.Visit(HeightSegments, "Height Segments");
        visitor.Visit(PhiStart, "Phi Start");
        visitor.Visit(PhiLength, "Phi Length");
        visitor.Visit(ThetaStart, "Theta Start");
        visitor.Visit(ThetaLength, "Theta Length");
        visitor.Visit(Material, "Material");
    }
    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.Visit(Radius, "Radius");
        visitor.Visit(WidthSegments, "Width Segments");
        visitor.Visit(HeightSegments, "Height Segments");
        visitor.Visit(PhiStart, "Phi Start");
        visitor.Visit(PhiLength, "Phi Length");
        visitor.Visit(ThetaStart, "Theta Start");
        visitor.Visit(ThetaLength, "Theta Length");
        visitor.Visit(Material, "Material");
    }

    // ===== Comparison Operators =====
    bool operator==(const FSphereParams& other) const { return Equals(other); }
    bool operator!=(const FSphereParams& other) const { return !(*this == other); }

    // ===== Type Info =====
    static const char* GetStaticTypeName() { return "Sphere"; }
};
