#include "ShaderCompiler.h"

// #include "Chozo/FileSystem/FileStream.h"
#include "GlslIncluder.h"
#include "RendererAPI.h"
#include "ShaderUtils.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <set>

DEFINE_LOG_CATEGORY(LogShaderCompiler);

namespace ShaderUtils {

static const std::string GetSPIRType(const spirv_cross::SPIRType& type) {
    std::string result;

    switch (type.basetype) {
    case spirv_cross::SPIRType::BaseType::Boolean:
        result = "bool";
        break;
    case spirv_cross::SPIRType::BaseType::Int:
        result = "int";
        break;
    case spirv_cross::SPIRType::BaseType::UInt:
        result = "uint";
        break;
    case spirv_cross::SPIRType::BaseType::Float:
        result = "float";
        break;
    case spirv_cross::SPIRType::BaseType::Double:
        result = "double";
        break;
    default:
        result = "unknown";
        break;
    }

    // If it's a vector or matrix, print the dimensions
    if (type.vecsize > 1 && type.columns == 1) {
        result = "Vec" + std::to_string(type.vecsize);
    } else if (type.columns > 1) {
        result = "Mat" + std::to_string(type.columns);
    }

    // Handle arrays
    if (!type.array.empty()) {
        result = "Array" + std::to_string(type.array[0]); // Print the size of the first dimension
        // If there are multiple dimensions, you can iterate through them
        for (size_t i = 1; i < type.array.size(); ++i) {
            CZ_LOG(LogShaderCompiler, Trace, "Array dimension {0} size: {1}", i, type.array[i]);
        }
    }

    return result;
}

static void ReflectSPIRReSource(const spirv_cross::Compiler& compiler,
                                const spirv_cross::Resource& resource,
                                FShaderReflection& reflection) {
    const auto& bufferType = compiler.get_type(resource.base_type_id);
    std::string bufferName = compiler.get_name(resource.id);
    uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
    uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t memberCount = bufferType.member_types.size();

    // CZ_LOG(LogShaderCompiler, Trace, "  Name = {0}", bufferName);
    // CZ_LOG(LogShaderCompiler, Trace, "  Size = {0}", bufferSize);
    // CZ_LOG(LogShaderCompiler, Trace, "  Binding = {0}", binding);
    // CZ_LOG(LogShaderCompiler, Trace, "  Members = {0}", memberCount);

    // Iterate over each member of the struct
    for (uint32_t i = 0; i < memberCount; i++) {
        std::string memberName =
            compiler.get_member_name(resource.base_type_id,
                                     i); // Get the name of the member (e.g., "ModelMatrix")
        const auto& memberType =
            compiler.get_type(bufferType.member_types[i]); // Get the type of the member

        // Size and offset of the member
        size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
        size_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
        std::string SPIRType = GetSPIRType(memberType);
        //
        // CZ_LOG(LogShaderCompiler, Trace, "    Member: {0}", memberName);
        // CZ_LOG(LogShaderCompiler, Trace, "    Type: {0}", SPIRType);
        // CZ_LOG(LogShaderCompiler, Trace, "    Size: {0}", memberSize);
        // CZ_LOG(LogShaderCompiler, Trace, "    Offset: {0}", memberOffset);

        FUniformInfo info;
        info.name = memberName;
        info.resourceName = bufferName;
        info.type = SPIRType;
        info.size = memberSize;
        info.location = memberOffset;

        reflection.uniforms.emplace_back(info);
        reflection.uniformLocations[info.name] = memberOffset;
    }
}
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
    uint32 kind = ChozoUtils::Shader::StageToKind(input.Stage);
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

FShaderReflection CShaderCompiler::Reflect() { return FShaderReflection{}; }

bool CShaderCompiler::Compile(const FShaderCreateInfo& info, FShaderCompilerOutput& output) {
    CZ_LOG(LogShaderCompiler, Trace, "Compiling Shader: {} in Stage: {}", info.Name,
           ChozoUtils::Shader::StageToString(info.Stage));

    FShaderCompilerInput input;
    input.VirtualPath = info.VirtualPath;
    input.Stage = info.Stage;
    input.Macros.Add(info.Definitions);

    bool success = CompileInternal(input, output);

    if (success) {
        CZ_LOG(LogShaderCompiler, Info, "Shader: {} Compiled", info.Name);
    } else {
        CZ_LOG(LogShaderCompiler, Error, "Failed to Compile Shader: {}", info.Name);
    }

    return success;
}

const std::string CShaderCompiler::GetOrLoadSource(const std::filesystem::path& sourcePath) {
    CZ_LOG(LogShaderCompiler, Trace, "Load shader source from: {}", sourcePath.string());

    std::string source;
    if (m_SourceCache.find(sourcePath) == m_SourceCache.end()) {
        // [Note] File I/O happens only once per path
        source = ChozoUtils::File::ReadTextFile(sourcePath);
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
    std::string source = GetOrLoadSource(sourcePath);
    if (source.empty())
        return false;

    PreProcess(input, source);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    uint32 kind = ChozoUtils::Shader::StageToKind(input.Stage);
    shaderc_shader_kind shadercKind = static_cast<shaderc_shader_kind>(kind);

    // Inject Macros
    for (const auto& [name, value] : input.Macros.GetMap()) {
        options.AddMacroDefinition(name, value);
    }

    // Setup Includer (Crucial!)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    // CompileGlslToSpv handles macros and includes internally
    // if options are set
    auto result =
        compiler.CompileGlslToSpv(source, shadercKind, sourcePath.string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        CZ_LOG(LogShaderCompiler, Error, "ShaderC Error: {0}", result.GetErrorMessage());
        output.bSucceeded = false;
        return false;
    }

    output.Binary = {result.cbegin(), result.cend()};
    // Perform Reflection (Optional: use SPIRV-Reflect library)
    // output.Reflection = Reflect(output.Binary);
    output.bSucceeded = true;

    // CZ_LOG(LogShaderCompiler, Trace, "{}", source);
    return true;
}
