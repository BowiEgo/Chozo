#include "VulkanShaderCompiler.h"
#include "FileUtils.h"
#include "ShaderUtils.h"

#include <shaderc/shaderc.hpp>

DEFINE_LOG_CATEGORY(LogVulkanShaderCompiler);

bool CVulkanShaderCompiler::CompileInternal(const FShaderCompilerInput &input,
                                            FShaderCompilerOutput &output) {
    std::filesystem::path sourcePath = VFS::Resolve(input.SourcePath);
    std::string source = ChozoUtils::File::ReadTextFile(sourcePath);
    CZ_LOG(LogVulkanShaderCompiler, Trace, "Source Path: {0}",
           sourcePath.string());
    CZ_LOG(LogVulkanShaderCompiler, Trace, "Source Size: {0} bytes",
           source.size());

    if (source.empty())
        return false;

    PreProcess(input, source);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    uint32 kind = ChozoUtils::Shader::ShaderStageToKind(input.Stage);
    shaderc_shader_kind shadercKind = static_cast<shaderc_shader_kind>(kind);

    // Inject Macros
    for (const auto &[name, value] : input.Macros.GetMap()) {
        options.AddMacroDefinition(name, value);
    }

    // Setup Includer (Crucial!)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    // CompileGlslToSpv handles macros and includes internally
    // if options are set
    auto result = compiler.CompileGlslToSpv(
        source, shadercKind, sourcePath.string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        CZ_LOG(LogVulkanShaderCompiler, Error, "ShaderC Error: {0}",
               result.GetErrorMessage());
        output.bSucceeded = false;
        return false;
    }

    output.Binary = {result.cbegin(), result.cend()};
    // Perform Reflection (Optional: use SPIRV-Reflect library)
    // output.Reflection = Reflect(output.Binary);
    output.bSucceeded = true;

    // CZ_LOG(LogVulkanShaderCompiler, Trace, "{}", source);
    return true;
}