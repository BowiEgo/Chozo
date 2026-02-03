#pragma once

#include "Core.h"
#include "ShaderTypes.h"

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogShaderCompiler, Info);

class RENDERCORE_API ShaderCompiler : public RefCounted {
public:
    virtual ~ShaderCompiler() = default;

    void PreProcess(const ShaderCompilerInput &input,
                    std::string &outProcessedSource);
    ShaderReflection Reflect();
    bool Compile(const ShaderCreateInfo &rep, ShaderCompilerOutput &vsOutput,
                 ShaderCompilerOutput &fsOutput);

    virtual bool CompileInternal(const ShaderCompilerInput &input,
                                 ShaderCompilerOutput &output) = 0;

    static Scope<ShaderCompiler> Create();

protected:
    // Store compiler settings, not instance data like source code
};
} // namespace Chozo
