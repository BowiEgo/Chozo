#pragma once

#include "RHIExport.h"
#include "RHIShader.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIPipeline, Info);

struct FRHIPipelineCreateInfo {
    std::string Name;
    std::vector<TRef<IRHIShader>> RHIShaders;

    std::vector<EPixelFormat> ColorFormats;
    EPixelFormat DepthFormat = EPixelFormat::D32_SFLOAT;
};

class RHI_API IRHIPipeline : public FRefCounted {
public:
    IRHIPipeline(const FRHIPipelineCreateInfo& info);
    virtual ~IRHIPipeline();

    virtual void Bind() = 0;

protected:
    FRHIPipelineCreateInfo m_Info;
};