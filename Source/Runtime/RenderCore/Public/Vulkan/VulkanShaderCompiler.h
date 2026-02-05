#pragma once

#include "ShaderCompiler.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanShaderCompiler, Info);

class RENDER_CORE_API CVulkanShaderCompiler : public CShaderCompiler {
public:
    CVulkanShaderCompiler() = default;
    ~CVulkanShaderCompiler() override = default;

    virtual bool CompileInternal(const FShaderCompilerInput &input,
                                 FShaderCompilerOutput &output) override;
};