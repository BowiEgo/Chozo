#pragma once

#include "Asset.h"
#include "Params.h"

struct FHDRIBackdropParams : public IParams {
    FAssetHandle Cubemap;
    float Intensity = 1.0f;
    float Lod       = 0.0f;

    FHDRIBackdropParams() = default;

    FHDRIBackdropParams(float intensity, float lod) : Intensity(intensity), Lod(lod) {}

    FHDRIBackdropParams(const FHDRIBackdropParams& other)
        : Cubemap(other.Cubemap), Intensity(other.Intensity), Lod(other.Lod) {}

    FHDRIBackdropParams& operator=(const FHDRIBackdropParams& other) {
        if (this != &other) {
            Cubemap   = other.Cubemap;
            Intensity = other.Intensity;
            Lod       = other.Lod;
        }
        return *this;
    }

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FHDRIBackdropParams(*this); }

    virtual bool Equals(const IParams& other) const override {
        const auto* otherParams = dynamic_cast<const FHDRIBackdropParams*>(&other);
        if (!otherParams) return false;

        return Cubemap == otherParams->Cubemap && Intensity == otherParams->Intensity &&
               Lod == otherParams->Lod;
    }

    virtual size_t GetHash() const override {
        size_t h = 0;
        HashCombine(h, std::hash<FAssetHandle>{}(Cubemap));
        HashCombine(h, std::hash<float>{}(Intensity));
        HashCombine(h, std::hash<float>{}(Lod));
        return h;
    }

    virtual std::string GetTypeName() const override { return "HDRIBackdrop"; }

    virtual size_t GetPropertyCount() const override { return 3; }
    virtual std::string GetPropertyName(size_t index) const override {
        static const std::string names[] = { "Cubemap", "Intensity", "Lod" };
        return index < GetPropertyCount() ? names[index] : "";
    }
    virtual std::any GetProperty(const std::string& name) const override {
        if (name == "Cubemap") return Cubemap;
        if (name == "Intensity") return Intensity;
        if (name == "Lod") return Lod;
        return {};
    }

    virtual void Accept(IParamsVisitor& visitor) override {
        visitor.Visit(Cubemap, "Cubemap");
        visitor.Visit(Intensity, "Intensity");
        visitor.Visit(Lod, "Lod");
    }
    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.Visit(Cubemap, "Cubemap");
        visitor.Visit(Intensity, "Intensity");
        visitor.Visit(Lod, "Lod");
    }

    // ===== Comparison Operators =====
    bool operator==(const FHDRIBackdropParams& other) const { return Equals(other); }
    bool operator!=(const FHDRIBackdropParams& other) const { return !(*this == other); }

    // ===== Type Info =====
    static const char* GetStaticTypeName() { return "HDRIBackdrop"; }
};
