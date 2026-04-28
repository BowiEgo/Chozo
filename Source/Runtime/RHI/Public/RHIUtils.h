#pragma once

#include "RHIExport.h"
#include "RHIShader.h"
#include "RHITypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIUtils, Info);

namespace ChozoUtils::RHI {

RHI_API bool IsDepthFormat(EPixelFormat format);

RHI_API std::map<uint32_t, FRHISetLayoutDescription>
    GenerateLayoutDescriptions(const FShaderReflection& reflection, EShaderStage currentStage);

RHI_API FRHIPipelineLayoutDescription
    GeneratePipelineLayoutDesc(const std::vector<TRef<IRHIShader>>& RHIShaders);

RHI_API EShaderDataType ToShaderDataFormat(EPixelFormat format);

} // namespace ChozoUtils::RHI