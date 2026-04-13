#pragma once

#include "Core.h"
#include "RHITypes.h"
#include "Shader.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShaderCompiler, Info);

class CShaderCompiler : public FRefCounted {
public:
    virtual ~CShaderCompiler() = default;

    bool Compile(const FShaderCompilerMultiInput& input,
                 std::unordered_map<EShaderStage, FShaderCompilerOutput>& outputs);

    FShaderReflection Reflect(const std::vector<uint32_t>& spirvBinary);

private:
    const std::string GetOrLoadSource(const std::filesystem::path& sourcePath);

    bool PreProcess(const std::filesystem::path& sourcePath, const EShaderStage stage,
                    std::string& outProcessedSource);

    bool CompileFromSource(const std::string& source, const std::filesystem::path& sourcePath,
                           const EShaderStage stage, const FShaderMacros& macros,
                           FShaderCompilerOutput& output);

protected:
    std::unordered_map<std::filesystem::path, std::string> m_SourceCache;
};
