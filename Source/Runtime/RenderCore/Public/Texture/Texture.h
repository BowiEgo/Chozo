#pragma once

#include "Asset.h"
#include "RHITexture.h"
#include "RenderCoreExport.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTexture, Info);

class RENDER_CORE_API CTexture : public IAsset {
public:
    CTexture(const FTextureSpecification& spec);
    CTexture(const FTextureSpecification& spec, const FBuffer& data);
    virtual ~CTexture() = default;

    virtual const std::string GetName() const override { return m_Spec.Name; }
    virtual const EAssetType GetType() const override { return EAssetType::Texture; }

    const FTextureSpecification& GetSpec() const { return m_Spec; }
    FExtent2D GetSize() const { return m_Spec.Size; }
    EPixelFormat GetFormat() const { return m_Spec.Format; }
    ETextureUsage GetUsage() const { return m_Spec.Usage; }

    IRHITexture* GetOrCreateResource();

protected:
    FTextureSpecification m_Spec;
    TScope<IRHITexture> m_Resource;

    FBuffer m_Data;
};
