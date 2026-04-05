#pragma once

#include "RHIShader.h"
#include "RHITypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIUtils, Info);

namespace ChozoUtils::RHI {

bool IsDepthFormat(EPixelFormat format);

std::map<uint32_t, FRHISetLayoutDescription>
    GenerateLayoutDescriptions(const FShaderReflection& reflection, EShaderStage currentStage);

FRHIPipelineLayoutDescription
    GeneratePipelineLayoutDesc(const std::vector<TRef<IRHIShader>>& RHIShaders);

} // namespace ChozoUtils::RHI