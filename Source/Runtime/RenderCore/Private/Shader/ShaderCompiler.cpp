#include "ShaderCompiler.h"

// #include "Chozo/FileSystem/FileStream.h"
#include "GlslIncluder.h"
#include "RendererAPI.h"
#include "ShaderUtils.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_reflect.h>

#include <set>

DEFINE_LOG_CATEGORY(LogShaderCompiler);

bool CShaderCompiler::Compile(const FShaderCompilerMultiInput& input,
                              std::unordered_map<EShaderStage, FShaderCompilerOutput>& outputs) {
    std::filesystem::path sourcePath = VFS::Resolve(input.VirtualPath);
    std::string source               = GetOrLoadSource(sourcePath);
    if (source.empty()) return false;

    bool allSuccess = true;
    for (EShaderStage stage : input.Stages) {
        FShaderCompilerOutput output;
        std::string stageSource = source;

        if (!PreProcess(sourcePath, stage, stageSource)) allSuccess = false;

        if (!CompileFromSource(stageSource, sourcePath, stage, input.Macros, output))
            allSuccess = false;

        // CZ_LOG(LogShaderCompiler, Trace, "StageSource: \n{}", stageSource);
        outputs[stage] = std::move(output);
    }

    return allSuccess;
}

FShaderReflection CShaderCompiler::Reflect(const std::vector<uint32_t>& spirvBinary) {
    FShaderReflection reflection;

    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(spirvBinary.size() * sizeof(uint32_t),
                                                           spirvBinary.data(), &module);
    CZ_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

    // --- 1. Reflect Input Attributes ---
    uint32_t inputCount = 0;
    spvReflectEnumerateInputVariables(&module, &inputCount, nullptr);
    std::vector<SpvReflectInterfaceVariable*> inputVars(inputCount);
    spvReflectEnumerateInputVariables(&module, &inputCount, inputVars.data());

    for (auto* var : inputVars) {
        if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) continue;

        EShaderDataFormat format = ChozoUtils::Shader::GetDataFormatFromSpv(*var->type_description);
        reflection.Attributes.emplace_back(var->name, var->location, format);
    }

    // --- 2. Reflect Descriptor Bindings ---
    uint32_t bindingCount = 0;
    spvReflectEnumerateDescriptorBindings(&module, &bindingCount, nullptr);
    std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
    spvReflectEnumerateDescriptorBindings(&module, &bindingCount, bindings.data());

    for (auto* binding : bindings) {
        FUniformSpecification spec;
        spec.Set     = binding->set;
        spec.Binding = binding->binding;
        spec.Name    = binding->name;
        spec.Type    = ChozoUtils::Shader::GetUniformTypeFromSpvDescType(binding->descriptor_type);
        spec.ArraySize = (binding->array.dims_count > 0) ? binding->array.dims[0] : 1;

        if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            spec.Size = binding->block.size;
        } else {
            spec.Size = 0;
        }

        reflection.Uniforms.push_back(spec);
        reflection.UniformLocations[spec.Name] = spec.Binding;
    }

    // --- 3. Reflect Push Constants ---
    uint32_t pcCount = 0;
    spvReflectEnumeratePushConstantBlocks(&module, &pcCount, nullptr);
    std::vector<SpvReflectBlockVariable*> pcBlocks(pcCount);
    spvReflectEnumeratePushConstantBlocks(&module, &pcCount, pcBlocks.data());

    for (auto* block : pcBlocks) {
        FUniformSpecification spec;
        spec.Type   = EUniformType::PushConstant;
        spec.Name   = block->name; // "VertexContant"
        spec.Size   = block->size; // 112 bytes (64 + 48)
        spec.Offset = block->offset;

        reflection.Uniforms.push_back(spec);
    }

    spvReflectDestroyShaderModule(&module);
    return reflection;
}

const std::string CShaderCompiler::GetOrLoadSource(const std::filesystem::path& sourcePath) {
    CZ_LOG(LogShaderCompiler, Trace, "Load shader source from: {}", sourcePath.string());

    std::string source;
    if (m_SourceCache.find(sourcePath) == m_SourceCache.end()) {
        // [Note] File I/O happens only once per path
        source                    = ChozoUtils::File::ReadTextFile(sourcePath);
        m_SourceCache[sourcePath] = source;
        CZ_LOG(LogShaderCompiler, Info, "Cached shader source: {}", sourcePath.string());
    } else {
        source = m_SourceCache[sourcePath];
    }

    CZ_LOG(LogShaderCompiler, Trace, "Source size: {} bytes", source.size());
    return source;
}

bool CShaderCompiler::PreProcess(const std::filesystem::path& sourcePath, const EShaderStage stage,
                                 std::string& outProcessedSource) {
    if (!std::filesystem::exists(sourcePath)) {
        CZ_LOG(LogShaderCompiler, Error, "Shader source file does not exist: {}",
               sourcePath.string());
        outProcessedSource.clear();
        return false;
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetIncluder(std::make_unique<FGlslIncluder>());

    std::string stageMacro = ChozoUtils::Shader::StageToString(stage, true) + "_SHADER";
    if (!stageMacro.empty()) {
        options.AddMacroDefinition(stageMacro);
    }

    uint32 kind                     = ChozoUtils::Shader::StageToKind(stage);
    shaderc_shader_kind shadercKind = static_cast<shaderc_shader_kind>(kind);

    auto result = compiler.PreprocessGlsl(outProcessedSource, shadercKind,
                                          sourcePath.string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        CZ_LOG(LogShaderCompiler, Error, "Failed to pre-process \"{}\".\nError: {}",
               sourcePath.string(), result.GetErrorMessage());
        outProcessedSource.clear();
        return false;
    } else {
        outProcessedSource = std::string(result.begin(), result.end());
    }

    return true;
}

bool CShaderCompiler::CompileFromSource(const std::string& source,
                                        const std::filesystem::path& sourcePath,
                                        const EShaderStage stage, const FShaderMacros& macros,
                                        FShaderCompilerOutput& output) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    shaderc_shader_kind shadercKind =
        static_cast<shaderc_shader_kind>(ChozoUtils::Shader::StageToKind(stage));

    std::string stageMacro = ChozoUtils::Shader::StageToString(stage, true) + "_SHADER";
    if (!stageMacro.empty()) {
        options.AddMacroDefinition(stageMacro);
    }

    for (const auto& [name, value] : macros.GetMap()) {
        options.AddMacroDefinition(name, value);
    }

    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetGenerateDebugInfo();

    auto result =
        compiler.CompileGlslToSpv(source, shadercKind, sourcePath.string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        CZ_LOG(LogShaderCompiler, Error, "Failed to compile stage {} for {}",
               ChozoUtils::Shader::StageToString(stage), sourcePath.string());
        output.bSucceeded = false;
        return false;
    }

    output.Binary     = { result.cbegin(), result.cend() };
    output.Reflection = Reflect(output.Binary);
    output.bSucceeded = true;
    return true;
}
