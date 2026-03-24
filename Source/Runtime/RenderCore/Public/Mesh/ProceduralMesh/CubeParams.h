#pragma once

#include "MeshTypes.h"
#include "Params.h"

struct FCubeParams : public IParams {
    float Width = 1.0f;
    float Height = 1.0f;
    float Depth = 1.0f;
    uint32_t WidthSegments = 1;
    uint32_t HeightSegments = 1;
    uint32_t DepthSegments = 1;

    FCubeParams() = default;

    FCubeParams(float width, float height, float depth, uint32_t widthSegments,
                uint32_t heightSegments, uint32_t depthSegments)
        : Width(width), Height(height), Depth(depth), WidthSegments(widthSegments),
          HeightSegments(heightSegments), DepthSegments(depthSegments) {}

    FCubeParams(const FCubeParams& other)
        : Width(other.Width), Height(other.Height), Depth(other.Depth),
          WidthSegments(other.WidthSegments), HeightSegments(other.HeightSegments),
          DepthSegments(other.DepthSegments) {}

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FCubeParams(*this); }

    virtual bool Equals(const IParams& other) const override {
        const auto* otherCube = dynamic_cast<const FCubeParams*>(&other);
        if (!otherCube) return false;

        return Width == otherCube->Width && Height == otherCube->Height &&
               Depth == otherCube->Depth && WidthSegments == otherCube->WidthSegments &&
               HeightSegments == otherCube->HeightSegments &&
               DepthSegments == otherCube->DepthSegments;
    }

    virtual size_t GetHash() const override {
        size_t seed = 0;
        seed ^= std::hash<float>{}(Width) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>{}(Height) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>{}(Depth) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<uint32_t>{}(WidthSegments) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<uint32_t>{}(HeightSegments) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<uint32_t>{}(DepthSegments) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

    virtual std::string GetTypeName() const override { return "Cube"; }

    virtual size_t GetPropertyCount() const override { return 6; }
    virtual std::string GetPropertyName(size_t index) const override {
        static const std::string names[] = { "Width",         "Height",         "Depth",
                                             "WidthSegments", "HeightSegments", "DepthSegments" };
        return index < GetPropertyCount() ? names[index] : "";
    }

    virtual void Accept(IParamsVisitor& visitor) override {
        visitor.Visit(Width, "Width");
        visitor.Visit(Height, "Height");
        visitor.Visit(Depth, "Depth");
        visitor.Visit(WidthSegments, "Width Segments");
        visitor.Visit(HeightSegments, "Height Segments");
        visitor.Visit(DepthSegments, "Depth Segments");
    }
    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.Visit(Width, "Width");
        visitor.Visit(Height, "Height");
        visitor.Visit(Depth, "Depth");
        visitor.Visit(WidthSegments, "Width Segments");
        visitor.Visit(HeightSegments, "Height Segments");
        visitor.Visit(DepthSegments, "Depth Segments");
    }

    // ===== Comparison Operators =====
    bool operator==(const FCubeParams& other) const { return Equals(other); }
    bool operator!=(const FCubeParams& other) const { return !(*this == other); }

    // ===== Type Info =====
    static const char* GetStaticTypeName() { return "Cube"; }
};
