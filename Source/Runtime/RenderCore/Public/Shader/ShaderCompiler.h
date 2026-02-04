#pragma once

#include "Core.h"
#include "ShaderTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShaderCompiler, Info);

class RENDERCORE_API CShaderCompiler : public FRefCounted {
public:
    virtual ~CShaderCompiler() = default;

    void PreProcess(const FShaderCompilerInput &input,
                    std::string &outProcessedSource);
    FShaderReflection Reflect();
    bool Compile(const FShaderCreateInfo &rep, FShaderCompilerOutput &vsOutput,
                 FShaderCompilerOutput &fsOutput);

    virtual bool CompileInternal(const FShaderCompilerInput &input,
                                 FShaderCompilerOutput &output) = 0;

    static TScope<CShaderCompiler> Create();

protected:
    // Store compiler settings, not instance data like source code
};
