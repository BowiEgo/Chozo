#pragma once

#include "RHIExport.h"
#include "RHISetLayout.h"
#include "RHIShader.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIPipeline, Info);

struct FPushConstantRange {
    uint32_t Offset;
    uint32_t Size;
    EShaderStage StageFlags;
};

struct FPipelineSpecification {
    std::string Name;
    std::vector<TRef<IRHIShader>> RHIShaders;

    std::unordered_map<uint32_t, TRef<IRHISetLayout>> RHISeyLayouts;
    VertexBufferLayout VertexLayout;
    std::vector<FPushConstantRange> PushConstantRanges;

    std::vector<EPixelFormat> OutputColorFormats;
    EPixelFormat DepthFormat  = EPixelFormat::D32F;
    EPolygonMode PolygonMode  = EPolygonMode::Fill;
    ECullMode CullMode        = ECullMode::Back;
    bool bDepthTestEnable     = true;
    bool bDepthWriteEnable    = true;
    ECompareOp DepthCompareOp = ECompareOp::Less;
};

class RHI_API IRHIPipeline : public FRefCounted {
public:
    IRHIPipeline(const FPipelineSpecification& spec);
    virtual ~IRHIPipeline();

    virtual const TRef<IRHISetLayout> GetSetLayout(uint32_t set) = 0;

    FPipelineSpecification GetSpec() const { return m_Spec; }
    EPolygonMode GetPolygonMode() const { return m_Spec.PolygonMode; }

protected:
    FPipelineSpecification m_Spec;
    std::vector<TRef<IRHISetLayout>> m_DescriptorSetLayouts;
};