#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Header/Types.h>
#include <Runtime/RHI/RHITypes.hpp>

#include <filesystem>

#include "slang-com-ptr.h"
#include "slang.h"

namespace CZ {

using Slang::ComPtr;

class ShaderCompiler {

public:
    static ShaderCompiler& Get();

    ShaderCompiler();
    ~ShaderCompiler() {};

    bool Compile(const ShaderCompilerMultiInput& input,
                 std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs);

private:
    bool CompileFromSource(slang::ISession* session, const std::string fileName,
                           std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs);

    bool CompileToSpirvForAllEntryPoints(
        ComPtr<slang::IComponentType>& shaderProgram, slang::ProgramLayout* programLayout,
        std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs);

    ShaderReflection ReflectFromProgramLayout(slang::ProgramLayout* programLayout);

    void PrintProgramLayout(slang::ProgramLayout* programLayout, SlangCompileTarget targetFormat);

private:
    std::unordered_map<std::filesystem::path, std::string> m_SourceCache;
    Slang::ComPtr<slang::IGlobalSession> m_GlobalSession;
};

} // namespace CZ
