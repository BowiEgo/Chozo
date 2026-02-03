#pragma once

#include "ShaderCompiler.h"

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanShaderCompiler, Info);

class RENDERCORE_API VulkanShaderCompiler : public ShaderCompiler {
public:
    VulkanShaderCompiler() = default;
    ~VulkanShaderCompiler() override = default;

    virtual bool CompileInternal(const ShaderCompilerInput &input,
                                 ShaderCompilerOutput &output) override;
};
} // namespace Chozo