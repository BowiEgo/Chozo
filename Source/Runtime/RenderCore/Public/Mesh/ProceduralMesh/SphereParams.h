#pragma once

#include "MathUtils.h"
#include "Params.h"

struct FSphereParams : public IParams {
    float Radius = 0.5f;
    uint32_t WidthSegments = 32;
    uint32_t HeightSegments = 16;
    float PhiStart = 0.0f;
    float PhiLength = ChozoUtils::Math::PI * 2;
    float ThetaStart = 0.0f;
    float ThetaLength = ChozoUtils::Math::PI;

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
        size_t seed = 0;
        seed ^= std::hash<float>{}(Radius) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<uint32_t>{}(WidthSegments) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<uint32_t>{}(HeightSegments) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>{}(PhiStart) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>{}(PhiLength) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>{}(ThetaStart) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>{}(ThetaLength) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

    virtual std::string GetTypeName() const override { return "Sphere"; }

    virtual size_t GetPropertyCount() const override { return 7; }
    virtual std::string GetPropertyName(size_t index) const override {
        static const std::string names[] = { "Radius",    "Segments",   "Rings",      "PhiStart",
                                             "PhiLength", "ThetaStart", "ThetaLength" };
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
    }
    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.Visit(Radius, "Radius");
        visitor.Visit(WidthSegments, "Width Segments");
        visitor.Visit(HeightSegments, "Height Segments");
        visitor.Visit(PhiStart, "Phi Start");
        visitor.Visit(PhiLength, "Phi Length");
        visitor.Visit(ThetaStart, "Theta Start");
        visitor.Visit(ThetaLength, "Theta Length");
    }

    // ===== Comparison Operators =====
    bool operator==(const FSphereParams& other) const { return Equals(other); }
    bool operator!=(const FSphereParams& other) const { return !(*this == other); }

    // ===== Type Info =====
    static const char* GetStaticTypeName() { return "Sphere"; }
};
