#pragma once

#include "RHIExport.h"
#include "RHIShader.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIPipeline, Info);

struct FPushConstantRange {
    uint32_t Offset;
    uint32_t Size;
};

struct FPipelineSpecification {
    std::string Name;
    std::vector<TRef<IRHIShader>> RHIShaders;

    std::vector<EPixelFormat> ColorFormats;
    EPixelFormat DepthFormat = EPixelFormat::D32_SFLOAT;

    VertexBufferLayout VertexLayout;
    std::vector<FPushConstantRange> PushConstantRanges;

    EPolygonMode PolygonMode = EPolygonMode::Fill;
};

class RHI_API IRHIPipeline : public FRefCounted {
public:
    IRHIPipeline(const FPipelineSpecification& spec);
    virtual ~IRHIPipeline();

    EPolygonMode GetPolygonMode() const { return m_Spec.PolygonMode; }

protected:
    FPipelineSpecification m_Spec;
};