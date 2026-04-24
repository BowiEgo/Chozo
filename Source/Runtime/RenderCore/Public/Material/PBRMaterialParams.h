#pragma once

#include "Params.h"

struct FPBRMaterialParams : public IParams {
    FVector3 Albedo               = FVector3(1.0f, 1.0f, 1.0f);
    float Metallic                = 0.0f;
    float Roughness               = 0.5f;
    float NormalStrength          = 1.0f;
    float EmissiveStrength        = 0.0f;
    bool BUseAlbedoMap            = false;
    bool BUseNormalMap            = false;
    bool BUseMetallicRoughnessMap = false;

    FPBRMaterialParams() = default;

    FPBRMaterialParams(const FVector3& albedo, float metallic, float roughness,
                       float normalStrength = 1.0f, float emissiveStrength = 0.0f,
                       bool bUseAlbedoMap = false, bool bUseNormalMap = false,
                       bool bUseMetallicRoughnessMap = false)
        : Albedo(albedo), Metallic(metallic), Roughness(roughness), NormalStrength(normalStrength),
          EmissiveStrength(emissiveStrength), BUseAlbedoMap(bUseAlbedoMap),
          BUseNormalMap(bUseNormalMap), BUseMetallicRoughnessMap(bUseMetallicRoughnessMap) {}

    FPBRMaterialParams(const FPBRMaterialParams& other)
        : Albedo(other.Albedo), Metallic(other.Metallic), Roughness(other.Roughness),
          NormalStrength(other.NormalStrength), EmissiveStrength(other.EmissiveStrength),
          BUseAlbedoMap(other.BUseAlbedoMap), BUseNormalMap(other.BUseNormalMap),
          BUseMetallicRoughnessMap(other.BUseMetallicRoughnessMap) {}

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FPBRMaterialParams(*this); }

    virtual bool Equals(const IParams& other) const override {
        const auto* otherMat = dynamic_cast<const FPBRMaterialParams*>(&other);
        if (!otherMat) return false;
        return Albedo == otherMat->Albedo && Metallic == otherMat->Metallic &&
               Roughness == otherMat->Roughness && NormalStrength == otherMat->NormalStrength &&
               EmissiveStrength == otherMat->EmissiveStrength &&
               BUseAlbedoMap == otherMat->BUseAlbedoMap &&
               BUseNormalMap == otherMat->BUseNormalMap &&
               BUseMetallicRoughnessMap == otherMat->BUseMetallicRoughnessMap;
    }

    virtual size_t GetHash() const override {
        size_t h = 0;
        HashCombine(h, std::hash<float>{}(Albedo.x));
        HashCombine(h, std::hash<float>{}(Albedo.y));
        HashCombine(h, std::hash<float>{}(Albedo.z));
        HashCombine(h, std::hash<float>{}(Metallic));
        HashCombine(h, std::hash<float>{}(Roughness));
        HashCombine(h, std::hash<float>{}(NormalStrength));
        HashCombine(h, std::hash<float>{}(EmissiveStrength));
        HashCombine(h, std::hash<bool>{}(BUseAlbedoMap));
        HashCombine(h, std::hash<bool>{}(BUseNormalMap));
        HashCombine(h, std::hash<bool>{}(BUseMetallicRoughnessMap));
        return h;
    }

    virtual std::string GetTypeName() const override { return "Material"; }

    virtual size_t GetPropertyCount() const override { return 8; }
    virtual std::string GetPropertyName(size_t index) const override {
        static const std::string names[] = { "Albedo",           "Metallic",
                                             "Roughness",        "NormalStrength",
                                             "EmissiveStrength", "UseAlbedoMap",
                                             "UseNormalMap",     "UseMetallicRoughnessMap" };
        return index < GetPropertyCount() ? names[index] : "";
    }

    virtual void Accept(IParamsVisitor& visitor) override {
        visitor.Visit(Albedo, "Albedo");
        visitor.Visit(Metallic, "Metallic");
        visitor.Visit(Roughness, "Roughness");
        visitor.Visit(NormalStrength, "Normal Strength");
        visitor.Visit(EmissiveStrength, "Emissive Strength");
        visitor.Visit(BUseAlbedoMap, "Use Albedo Map");
        visitor.Visit(BUseNormalMap, "Use Normal Map");
        visitor.Visit(BUseMetallicRoughnessMap, "Use Metallic Roughness Map");
    }

    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.Visit(Albedo, "Albedo");
        visitor.Visit(Metallic, "Metallic");
        visitor.Visit(Roughness, "Roughness");
        visitor.Visit(NormalStrength, "Normal Strength");
        visitor.Visit(EmissiveStrength, "Emissive Strength");
        visitor.Visit(BUseAlbedoMap, "Use Albedo Map");
        visitor.Visit(BUseNormalMap, "Use Normal Map");
        visitor.Visit(BUseMetallicRoughnessMap, "Use Metallic Roughness Map");
    }

    bool operator==(const FPBRMaterialParams& other) const { return Equals(other); }
    bool operator!=(const FPBRMaterialParams& other) const { return !(*this == other); }

    static const char* GetStaticTypeName() { return "Material"; }
};