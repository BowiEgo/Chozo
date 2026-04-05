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

namespace ShaderUtils {

// static const std::string GetSPIRType(const spirv_cross::SPIRType& type) {
//     std::string result;

//     switch (type.basetype) {
//         case spirv_cross::SPIRType::BaseType::Boolean: result = "bool"; break;
//         case spirv_cross::SPIRType::BaseType::Int: result = "int"; break;
//         case spirv_cross::SPIRType::BaseType::UInt: result = "uint"; break;
//         case spirv_cross::SPIRType::BaseType::Float: result = "float"; break;
//         case spirv_cross::SPIRType::BaseType::Double: result = "double"; break;
//         default: result = "unknown"; break;
//     }

//     // If it's a vector or matrix, print the dimensions
//     if (type.vecsize > 1 && type.columns == 1) {
//         result = "Vec" + std::to_string(type.vecsize);
//     } else if (type.columns > 1) {
//         result = "Mat" + std::to_string(type.columns);
//     }

//     // Handle arrays
//     if (!type.array.empty()) {
//         result = "Array" + std::to_string(type.array[0]); // Print the size of the first
//         dimension
//         // If there are multiple dimensions, you can iterate through them
//         for (size_t i = 1; i < type.array.size(); ++i) {
//             CZ_LOG(LogShaderCompiler, Trace, "Array dimension {0} size: {1}", i, type.array[i]);
//         }
//     }

//     return result;
// }

// static void ReflectSPIRReSource(const spirv_cross::Compiler& compiler,
//                                 const spirv_cross::Resource& resource,
//                                 FShaderReflection& reflection) {
//     const auto& bufferType = compiler.get_type(resource.base_type_id);
//     std::string bufferName = compiler.get_name(resource.id);
//     uint32_t bufferSize    = compiler.get_declared_struct_size(bufferType);
//     uint32_t binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
//     uint32_t memberCount   = bufferType.member_types.size();

//     // CZ_LOG(LogShaderCompiler, Trace, "  Name = {0}", bufferName);
//     // CZ_LOG(LogShaderCompiler, Trace, "  Size = {0}", bufferSize);
//     // CZ_LOG(LogShaderCompiler, Trace, "  Binding = {0}", binding);
//     // CZ_LOG(LogShaderCompiler, Trace, "  Members = {0}", memberCount);

//     // Iterate over each member of the struct
//     for (uint32_t i = 0; i < memberCount; i++) {
//         std::string memberName =
//             compiler.get_member_name(resource.base_type_id,
//                                      i); // Get the name of the member (e.g., "ModelMatrix")
//         const auto& memberType =
//             compiler.get_type(bufferType.member_types[i]); // Get the type of the member

//         // Size and offset of the member
//         size_t memberSize    = compiler.get_declared_struct_member_size(bufferType, i);
//         size_t memberOffset  = compiler.type_struct_member_offset(bufferType, i);
//         std::string SPIRType = GetSPIRType(memberType);
//         //
//         // CZ_LOG(LogShaderCompiler, Trace, "    Member: {0}", memberName);
//         // CZ_LOG(LogShaderCompiler, Trace, "    Type: {0}", SPIRType);
//         // CZ_LOG(LogShaderCompiler, Trace, "    Size: {0}", memberSize);
//         // CZ_LOG(LogShaderCompiler, Trace, "    Offset: {0}", memberOffset);

//         FUniformSpecification spec;
//         spec.Name         = memberName;
//         spec.ResourceName = bufferName;
//         spec.Type         = SPIRType;
//         spec.Size         = memberSize;
//         spec.Location     = memberOffset;

//         reflection.Uniforms.emplace_back(spec);
//         reflection.UniformLocations[spec.Name] = memberOffset;
//     }
// }
} // namespace ShaderUtils

void CShaderCompiler::PreProcess(const FShaderCompilerInput& input,
                                 std::string& outProcessedSource) {
    std::filesystem::path shaderSourcePath = VFS::Resolve(input.VirtualPath);

    if (!std::filesystem::exists(shaderSourcePath)) {
        CZ_LOG(LogShaderCompiler, Error, "Shader source file does not exist: {}",
               shaderSourcePath.string());
        return;
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    uint32 kind                     = ChozoUtils::Shader::StageToKind(input.Stage);
    shaderc_shader_kind shadercKind = static_cast<shaderc_shader_kind>(kind);

    if (input.Stage == EShaderStage::Vertex) {
        options.AddMacroDefinition("VERTEX_SHADER");
    } else if (input.Stage == EShaderStage::Fragment) {
        options.AddMacroDefinition("FRAGMENT_SHADER");
    }

    options.SetIncluder(std::make_unique<FGlslIncluder>());

    const auto preProcessingResult = compiler.PreprocessGlsl(
        outProcessedSource, shadercKind, shaderSourcePath.string().c_str(), options);

    if (preProcessingResult.GetCompilationStatus() != shaderc_compilation_status_success)
        CZ_LOG(LogShaderCompiler, Error, "Failed to pre-process \"{}\"'s {} shader.\nError: {}",
               shaderSourcePath.string(), kind, preProcessingResult.GetErrorMessage());

    outProcessedSource = std::string(preProcessingResult.begin(), preProcessingResult.end());
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

        // 修正：确保能够解析出 Format。如果 Format 为 None，说明 GetDataFormatFromSpv 逻辑要加强
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
        spec.Name    = binding->name; // 这应该是 "CameraData" 而不是内部成员
        spec.Type    = ChozoUtils::Shader::GetUniformTypeFromSpvDescType(binding->descriptor_type);
        spec.ArraySize = (binding->array.dims_count > 0) ? binding->array.dims[0] : 1;

        if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            // 整个 Block 的大小
            spec.Size = binding->block.size;
        } else {
            spec.Size = 0; // 贴图等资源没有物理字节大小（在 Buffer 层面）
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

bool CShaderCompiler::Compile(const FShaderSpecification& spec, FShaderCompilerOutput& output) {
    CZ_LOG(LogShaderCompiler, Trace, "Compiling Shader: {} in Stage: {}", spec.Name,
           ChozoUtils::Shader::StageToString(spec.Stage));

    FShaderCompilerInput input;
    input.VirtualPath = spec.VirtualPath;
    input.Stage       = spec.Stage;
    input.Macros.Add(spec.Definitions);

    bool success = CompileInternal(input, output);

    if (success) {
        CZ_LOG(LogShaderCompiler, Info, "Shader: {} Compiled", spec.Name);
    } else {
        CZ_LOG(LogShaderCompiler, Error, "Failed to Compile Shader: {}", spec.Name);
    }

    return success;
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

bool CShaderCompiler::CompileInternal(const FShaderCompilerInput& input,
                                      FShaderCompilerOutput& output) {
    const std::filesystem::path sourcePath = VFS::Resolve(input.VirtualPath);
    std::string source                     = GetOrLoadSource(sourcePath);
    if (source.empty()) return false;

    PreProcess(input, source);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    uint32 kind                     = ChozoUtils::Shader::StageToKind(input.Stage);
    shaderc_shader_kind shadercKind = static_cast<shaderc_shader_kind>(kind);

    // Inject Macros
    for (const auto& [name, value] : input.Macros.GetMap()) {
        options.AddMacroDefinition(name, value);
    }

    // Setup Includer (Crucial!)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetGenerateDebugInfo();

    // CompileGlslToSpv handles macros and includes internally
    // if options are set
    auto result =
        compiler.CompileGlslToSpv(source, shadercKind, sourcePath.string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        CZ_LOG(LogShaderCompiler, Error, "ShaderC Error: {0}", result.GetErrorMessage());
        output.bSucceeded = false;
        return false;
    }

    output.Binary     = { result.cbegin(), result.cend() };
    // Perform Reflection (Optional: use SPIRV-Reflect library)
    output.Reflection = Reflect(output.Binary);
    output.bSucceeded = true;

    // CZ_LOG(LogShaderCompiler, Trace, "{}", source);
    return true;
}
