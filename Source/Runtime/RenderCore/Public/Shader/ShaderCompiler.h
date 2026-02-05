#pragma once

#include "Core.h"
#include "RHITypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShaderCompiler, Info);

class CShaderCompiler : public FRefCounted {
public:
    virtual ~CShaderCompiler() = default;

    void PreProcess(const FShaderCompilerInput& input,
                    std::string& outProcessedSource);
    FShaderReflection Reflect();
    bool Compile(const FShaderCreateInfo& rep, FShaderCompilerOutput& vsOutput,
                 FShaderCompilerOutput& fsOutput);

private:
    bool CompileInternal(const FShaderCompilerInput& input,
                         FShaderCompilerOutput& output);

protected:
    // Store compiler settings, not instance data like source code
};
