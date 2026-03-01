#pragma once

#include "Core.h"
#include "RHITypes.h"
#include "Shader.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShaderCompiler, Info);

class CShaderCompiler : public FRefCounted {
public:
    virtual ~CShaderCompiler() = default;

    void PreProcess(const FShaderCompilerInput& input, std::string& outProcessedSource);
    FShaderReflection Reflect();
    bool Compile(const FShaderSpecification& spec, FShaderCompilerOutput& output);

private:
    const std::string GetOrLoadSource(const std::filesystem::path& sourcePath);
    bool CompileInternal(const FShaderCompilerInput& input, FShaderCompilerOutput& output);

protected:
    std::unordered_map<std::filesystem::path, std::string> m_SourceCache;
};
