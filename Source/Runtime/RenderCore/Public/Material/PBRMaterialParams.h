#pragma once

#include "AssetManager.h"
#include "MaterialParams.h"

struct RENDER_CORE_API FPBRMaterialParams : public IMaterialParams {
    FVector4 BaseColor     = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    float Metallic         = 0.0f;
    float Roughness        = 0.5f;
    float NormalStrength   = 1.0f;
    float EmissiveStrength = 0.0f;
    FAssetHandle AlbedoMap = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle NormalMap = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle RMAOMap   = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    bool UseAlbedoMap      = true;
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

    // #define PARAM(type, member, ...) , member(other.member)
    //     FPBRMaterialParams(const FPBRMaterialParams& other) : IMaterialParams(other)
    //     PBR_PARAMS_LIST {}
    // #undef PARAM

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FPBRMaterialParams(*this); }

    virtual size_t GetHash() const override;

    virtual std::any GetParamValue(const std::string& name) const override;

    // ===== IMaterialParams Implementation =====
    virtual bool Equals_Internal(const IParams& other) const override;

    virtual void Accept_Internal(IParamsVisitor& visitor) override;

    virtual void Accept_Internal(IConstParamsVisitor& visitor) const override;

    virtual const std::vector<std::string>& GetAllParamNames_Internal() override;

    // ===== Type Info =====
    virtual std::string GetTypeName() const override { return "PBR"; }
    static const char* GetStaticTypeName() { return "PBR"; }

    // ===== Comparison Operators =====
    bool operator==(const FPBRMaterialParams& other) const { return Equals(other); }
    bool operator!=(const FPBRMaterialParams& other) const { return !(*this == other); }
};
