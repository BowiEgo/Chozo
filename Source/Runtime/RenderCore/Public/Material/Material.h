#pragma once

#include "Asset.h"
#include "MaterialParamsWrapper.h"
#include "Params.h"
#include "RHIContext.h"
#include "RHIShader.h"
#include "Ref.h"
#include "RenderCoreExport.h"
#include "Shader.h"
#include "Texture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMaterial, Info);

struct FMaterialSpecification {
    std::string Name;
    TRef<CShader> Shader;
    std::vector<EPixelFormat> ColorFormats;
    EPixelFormat DepthFormat = EPixelFormat::D32_SFLOAT;
};

class RENDER_CORE_API CMaterial : public IAsset {
public:
    CMaterial(const FMaterialSpecification& spec, const FMaterialParamsWrapper& params)
        : m_Spec(spec), m_Params(params) {};
    virtual ~CMaterial() = default;

    virtual const std::string GetName() const override { return m_Spec.Name; }
    virtual const EAssetType GetType() const override { return EAssetType::Material; }

    const FMaterialSpecification GetSpec() const { return m_Spec; }
    TRef<CShader> GetShader() const { return m_Spec.Shader; }

    FMaterialParamsWrapper& GetParams() { return m_Params; }
    const FMaterialParamsWrapper& GetParams() const { return m_Params; }
    const TRef<IRHIPipeline> GetPipeline();

    void MarkDirty() { m_bIsDirty = true; }
    void CreateDescriptorSet();
    void Bind(IRHICommandList* cmdList);

private:
    FMaterialSpecification m_Spec;
    FMaterialParamsWrapper m_Params;

    TRef<IRHIPipeline> m_Pipeline;
    TRef<IRHIBuffer> m_UniformBuffer;

    bool m_bIsDirty = true;
    TRef<IRHIDescriptorSet> m_DescriptorSet;
};
