#pragma once

#include <Core/FileSystem/VFS.hpp>
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

    /**
     * @brief  Compiles a shader from the given input parameters.
     *
     * This is a convenience wrapper around CompileShader(). It takes the full
     * compilation input (virtual path, macros, etc.) and produces per-stage
     * SPIR‑V binaries. Internally, each call creates an independent slang::ISession,
     * making it safe for multi‑threaded compilation.
     *
     * @param input    Combined compilation input (virtual path, macros, etc.).
     * @param outputs  Map filled with the compiled binary for each requested shader stage.
     * @return         true if all requested stages compiled successfully, false otherwise.
     *
     * @note  This function is thread‑safe as long as the underlying file system
     *        and logging are thread‑safe.
     * @see   CompileShader(), ShaderCompilerMultiInput
     */
    bool Compile(const ShaderCompilerMultiInput& input,
                 std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs) {
        return CompileShader(input, outputs);
    }

    bool CompileShader(const ShaderCompilerMultiInput& input,
                       std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs) {
        std::filesystem::path sourcePath = VFS::Resolve(input.VirtualPath);
        std::string fileName             = sourcePath.filename().string();

        ComPtr<slang::ISession> session = CreateSession(input);
        if (!session) return false;

        return CompileFromSource(session, fileName, outputs);
    }

private:
    ComPtr<slang::ISession> CreateSession(const ShaderCompilerMultiInput& input);

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
