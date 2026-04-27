#pragma once

#include "MaterialParams.h"

struct FPBRMaterialParams : public IMaterialParams {
    FVector4 BaseColor     = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    float Metallic         = 0.0f;
    float Roughness        = 0.5f;
    float NormalStrength   = 1.0f;
    float EmissiveStrength = 0.0f;
    FAssetHandle AlbedoMap = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle NormalMap = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle RMAOMap   = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    bool UseAlbedoMap      = false;
    bool UseNormalMap      = false;
    bool UseRMAOMap        = false;

    FPBRMaterialParams() = default;

    FPBRMaterialParams(const FVector4& baseColor, float metallic, float roughness,
                       float normalStrength = 1.0f, float emissiveStrength = 0.0f,
                       bool useAlbedoMap = false, bool useNormalMap = false,
                       bool useRMAOMap = false)
        : BaseColor(baseColor), Metallic(metallic), Roughness(roughness),
          NormalStrength(normalStrength), EmissiveStrength(emissiveStrength),
          UseAlbedoMap(useAlbedoMap), UseNormalMap(useNormalMap), UseRMAOMap(useRMAOMap) {}

    FPBRMaterialParams(const FPBRMaterialParams& other)
        : BaseColor(other.BaseColor), Metallic(other.Metallic), Roughness(other.Roughness),
          NormalStrength(other.NormalStrength), EmissiveStrength(other.EmissiveStrength),
          UseAlbedoMap(other.UseAlbedoMap), UseNormalMap(other.UseNormalMap),
          UseRMAOMap(other.UseRMAOMap) {}

    // ===== IMaterialParams Implementation =====
    virtual IParams* Clone() const override { return new FPBRMaterialParams(*this); }

    virtual bool Equals(const IParams& other) const override {
        const auto* otherMat = dynamic_cast<const FPBRMaterialParams*>(&other);
        if (!otherMat) return false;
        return PolygonMode == otherMat->PolygonMode && CullMode == otherMat->CullMode &&
               BaseColor == otherMat->BaseColor && Metallic == otherMat->Metallic &&
               Roughness == otherMat->Roughness && NormalStrength == otherMat->NormalStrength &&
               EmissiveStrength == otherMat->EmissiveStrength &&
               UseAlbedoMap == otherMat->UseAlbedoMap && UseNormalMap == otherMat->UseNormalMap &&
               UseRMAOMap == otherMat->UseRMAOMap;
    }

    virtual size_t GetHash() const override {
        size_t h = 0;
        HashCombine(h, std::hash<EPolygonMode>{}(PolygonMode));
        HashCombine(h, std::hash<ECullMode>{}(CullMode));
        HashCombine(h, std::hash<float>{}(BaseColor.x));
        HashCombine(h, std::hash<float>{}(BaseColor.y));
        HashCombine(h, std::hash<float>{}(BaseColor.z));
        HashCombine(h, std::hash<float>{}(BaseColor.w));
        HashCombine(h, std::hash<float>{}(Metallic));
        HashCombine(h, std::hash<float>{}(Roughness));
        HashCombine(h, std::hash<float>{}(NormalStrength));
        HashCombine(h, std::hash<float>{}(EmissiveStrength));
        HashCombine(h, std::hash<FAssetHandle>{}(AlbedoMap));
        HashCombine(h, std::hash<FAssetHandle>{}(NormalMap));
        HashCombine(h, std::hash<FAssetHandle>{}(RMAOMap));
        HashCombine(h, std::hash<bool>{}(UseAlbedoMap));
        HashCombine(h, std::hash<bool>{}(UseNormalMap));
        HashCombine(h, std::hash<bool>{}(UseRMAOMap));
        return h;
    }

    virtual std::string GetTypeName() const override { return "PBR"; }

    virtual size_t GetPropertyCount() const override { return 13; }
    virtual std::string GetPropertyName(size_t index) const override {
        static const std::string names[] = { "PolygonMode",      "CullMode",     "BaseColor",
                                             "Metallic",         "Roughness",    "NormalStrength",
                                             "EmissiveStrength", "AlbedoMap",    "NormalMap",
                                             "RMAOMap",          "UseAlbedoMap", "UseNormalMap",
                                             "UseRMAOMap" };
        return index < GetPropertyCount() ? names[index] : "";
    }
    virtual std::any GetProperty(const std::string& name) const override {
        if (name == "PolygonMode") return PolygonMode;
        if (name == "CullMode") return CullMode;
        if (name == "BaseColor") return BaseColor;
        if (name == "Metallic") return Metallic;
        if (name == "Roughness") return Roughness;
        if (name == "NormalStrength") return NormalStrength;
        if (name == "EmissiveStrength") return EmissiveStrength;
        if (name == "AlbedoMap") return AlbedoMap;
        if (name == "NormalMap") return NormalMap;
        if (name == "RMAOMap") return RMAOMap;
        if (name == "UseAlbedoMap") return UseAlbedoMap;
        if (name == "UseNormalMap") return UseNormalMap;
        if (name == "UseRMAOMap") return UseRMAOMap;
        return {};
    }

    virtual void Accept(IParamsVisitor& visitor) override {
        uint32_t polygonMode = static_cast<uint32_t>(PolygonMode);
        visitor.Visit(polygonMode, "PolygonMode");
        PolygonMode = static_cast<EPolygonMode>(polygonMode);

        uint32_t cullMode = static_cast<uint32_t>(CullMode);
        visitor.Visit(cullMode, "CullMode");
        CullMode = static_cast<ECullMode>(cullMode);

        visitor.Visit(BaseColor, "BaseColor");
        visitor.Visit(Metallic, "Metallic");
        visitor.Visit(Roughness, "Roughness");
        visitor.Visit(NormalStrength, "Normal Strength");
        visitor.Visit(EmissiveStrength, "Emissive Strength");

        visitor.Visit(AlbedoMap, "AlbedoMap");
        visitor.Visit(NormalMap, "NormalMap");
        visitor.Visit(RMAOMap, "RMAOMap");

        visitor.Visit(UseAlbedoMap, "Use Albedo Map");
        visitor.Visit(UseNormalMap, "Use Normal Map");
        visitor.Visit(UseRMAOMap, "Use RMAO Map");
    }

    virtual void Accept(IConstParamsVisitor& visitor) const override {
        uint32_t polygonMode = static_cast<uint32_t>(PolygonMode);
        visitor.Visit(polygonMode, "PolygonMode");

        uint32_t cullMode = static_cast<uint32_t>(CullMode);
        visitor.Visit(cullMode, "CullMode");

        visitor.Visit(BaseColor, "BaseColor");
        visitor.Visit(Metallic, "Metallic");
        visitor.Visit(Roughness, "Roughness");
        visitor.Visit(NormalStrength, "Normal Strength");
        visitor.Visit(EmissiveStrength, "Emissive Strength");

        visitor.Visit(AlbedoMap, "AlbedoMap");
        visitor.Visit(NormalMap, "NormalMap");
        visitor.Visit(RMAOMap, "RMAOMap");

        visitor.Visit(UseAlbedoMap, "Use Albedo Map");
        visitor.Visit(UseNormalMap, "Use Normal Map");
        visitor.Visit(UseRMAOMap, "Use RMAO Map");
    }

    bool operator==(const FPBRMaterialParams& other) const { return Equals(other); }
    bool operator!=(const FPBRMaterialParams& other) const { return !(*this == other); }

    static const char* GetStaticTypeName() { return "Material"; }
};