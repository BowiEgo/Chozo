#pragma once

#include "AssetManager.h"
#include "MaterialParams.h"

struct RENDER_CORE_API FPBRMaterialParams : public IMaterialParams {
    FVector4 BaseColor       = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    float Metallic           = 0.0f;
    float Roughness          = 0.5f;
    float AOIntensity        = 1.0f;
    FVector3 EmissiveColor   = FVector3(0.0f, 0.0f, 0.0f);
    float EmissiveIntensity  = 0.0f;
    FAssetHandle AlbedoMap   = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle NormalMap   = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle RMAOMap     = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    FAssetHandle EmissiveMap = CAssetManager::Get().GetCheckboardTexture()->GetHandle();
    bool UseAlbedoMap        = true;
    bool UseNormalMap        = false;
    bool UseRMAOMap          = false;
    bool UseEmissiveMap      = false;

    FPBRMaterialParams() = default;

    FPBRMaterialParams(const FVector4& baseColor, float metallic = 0.0f, float roughness = 0.5f,
                       float aoIntensity             = 1.0f,
                       const FVector3& emissiveColor = FVector3(0.0f, 0.0f, 0.0f),
                       float emissiveIntensity = 0.0f, bool useAlbedoMap = false,
                       bool useNormalMap = false, bool useRMAOMap = false,
                       bool useEmissiveMap = false)
        : BaseColor(baseColor), Metallic(metallic), Roughness(roughness), AOIntensity(aoIntensity),
          EmissiveColor(emissiveColor), EmissiveIntensity(emissiveIntensity),
          UseAlbedoMap(useAlbedoMap), UseNormalMap(useNormalMap), UseRMAOMap(useRMAOMap),
          UseEmissiveMap(useEmissiveMap) {}

    FPBRMaterialParams(const FPBRMaterialParams& other)
        : BaseColor(other.BaseColor), Metallic(other.Metallic), Roughness(other.Roughness),
          AOIntensity(other.AOIntensity), EmissiveColor(other.EmissiveColor),
          EmissiveIntensity(other.EmissiveIntensity), UseAlbedoMap(other.UseAlbedoMap),
          UseNormalMap(other.UseNormalMap), UseRMAOMap(other.UseRMAOMap),
          UseEmissiveMap(other.UseEmissiveMap) {}

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
