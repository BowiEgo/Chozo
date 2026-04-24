#pragma once

#include "Asset.h"
#include "MaterialParams.h"
#include "Params.h"
#include "RHIContext.h"
#include "RHIShader.h"
#include "Ref.h"
#include "RenderCoreExport.h"
#include "Texture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMaterial, Info);

struct FMaterialSpecification {
    std::string Name;
};

class RENDER_CORE_API CMaterial : public IAsset {
public:
    CMaterial(const FMaterialSpecification& spec, const FMaterialParams& params)
        : m_Spec(spec), m_Params(params) {};
    virtual ~CMaterial() = default;

    virtual const std::string GetName() const override { return m_Spec.Name; }
    virtual const EAssetType GetType() const override { return EAssetType::Material; }

    FMaterialParams& GetParams() { return m_Params; }
    const FMaterialParams& GetParams() const { return m_Params; }

protected:
    const FMaterialSpecification m_Spec;
    FMaterialParams m_Params;
};
