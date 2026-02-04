#include "ShaderCompiler.h"

// #include "Chozo/FileSystem/FileStream.h"
#include "GlslIncluder.h"
#include "OpenGLShaderCompiler.h"
#include "RendererAPI.h"
#include "ShaderUtils.h"
#include "VulkanShaderCompiler.h"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <set>

DEFINE_LOG_CATEGORY(LogShaderCompiler);

TScope<CShaderCompiler> CShaderCompiler::Create() {
    switch (CRendererAPI::GetType()) {
    case CRendererAPI::EType::None:
        CZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
    case CRendererAPI::EType::OpenGL:
        return CreateScope<COpenGLShaderCompiler>();
    case CRendererAPI::EType::Vulkan:
        return CreateScope<CVulkanShaderCompiler>();
    }

    CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

namespace ShaderUtils {

static const std::string GetSPIRType(const spirv_cross::SPIRType &type) {
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
        result = "Array" +
                 std::to_string(
                     type.array[0]); // Print the size of the first dimension
        // If there are multiple dimensions, you can iterate through them
        for (size_t i = 1; i < type.array.size(); ++i) {
            CZ_LOG(LogShaderCompiler, Trace, "Array dimension {0} size: {1}", i,
                   type.array[i]);
        }
    }

    return result;
}

static void ReflectSPIRReSource(const spirv_cross::Compiler &compiler,
                                const spirv_cross::Resource &resource,
                                FShaderReflection &reflection) {
    const auto &bufferType = compiler.get_type(resource.base_type_id);
    std::string bufferName = compiler.get_name(resource.id);
    uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
    uint32_t binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t memberCount = bufferType.member_types.size();

    // CZ_LOG(LogShaderCompiler, Trace, "  Name = {0}", bufferName);
    // CZ_LOG(LogShaderCompiler, Trace, "  Size = {0}", bufferSize);
    // CZ_LOG(LogShaderCompiler, Trace, "  Binding = {0}", binding);
    // CZ_LOG(LogShaderCompiler, Trace, "  Members = {0}", memberCount);

    // Iterate over each member of the struct
    for (uint32_t i = 0; i < memberCount; i++) {
        std::string memberName = compiler.get_member_name(
            resource.base_type_id,
            i); // Get the name of the member (e.g., "ModelMatrix")
        const auto &memberType = compiler.get_type(
            bufferType.member_types[i]); // Get the type of the member

        // Size and offset of the member
        size_t memberSize =
            compiler.get_declared_struct_member_size(bufferType, i);
        size_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
        std::string SPIRType = GetSPIRType(memberType);
        //
        // CZ_LOG(LogShaderCompiler, Trace, "    Member: {0}", memberName);
        // CZ_LOG(LogShaderCompiler, Trace, "    Type: {0}", SPIRType);
        // CZ_LOG(LogShaderCompiler, Trace, "    Size: {0}", memberSize);
        // CZ_LOG(LogShaderCompiler, Trace, "    Offset: {0}", memberOffset);

        UniformInfo info;
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

void CShaderCompiler::PreProcess(const FShaderCompilerInput &input,
                                 std::string &outProcessedSource) {
    std::filesystem::path shaderSourcePath = VFS::Resolve(input.SourcePath);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    if (input.Stage == ShaderStage::Vertex) {
        options.AddMacroDefinition("VERTEX_SHADER");
    } else if (input.Stage == ShaderStage::Fragment) {
        options.AddMacroDefinition("FRAGMENT_SHADER");
    }

    options.SetIncluder(std::make_unique<FGlslIncluder>());

    const auto preProcessingResult = compiler.PreprocessGlsl(
        outProcessedSource, ChozoUtils::Shader::ShaderStageToKind(input.Stage),
        shaderSourcePath.string().c_str(), options);

    if (preProcessingResult.GetCompilationStatus() !=
        shaderc_compilation_status_success)
        CZ_CORE_ASSERT(
            "Renderer",
            fmt::format("Failed to pre-process \"{}\"'s {} shader.\nError: {}",
                        shaderSourcePath,
                        ChozoUtils::Shader::ShaderStageToString(input.Stage),
                        preProcessingResult.GetErrorMessage()));

    outProcessedSource =
        std::string(preProcessingResult.begin(), preProcessingResult.end());
}

FShaderReflection CShaderCompiler::Reflect() { return FShaderReflection{}; }

bool CShaderCompiler::Compile(const FShaderCreateInfo &rep,
                              FShaderCompilerOutput &vsOutput,
                              FShaderCompilerOutput &fsOutput) {
    CZ_LOG(LogShaderCompiler, Trace, "Compiling Shader: {}", rep.Name);

    bool sucess;

    FShaderCompilerInput vsInput;
    vsInput.SourcePath = rep.VirtualPath;
    vsInput.Stage = ShaderStage::Vertex;
    vsInput.Macros.Add(rep.Definitions);

    FShaderCompilerInput fsInput;
    fsInput.SourcePath = rep.VirtualPath;
    fsInput.Stage = ShaderStage::Fragment;
    fsInput.Macros.Add(rep.Definitions);

    bool vsSuccess = CompileInternal(vsInput, vsOutput);
    bool fsSuccess = CompileInternal(fsInput, fsOutput);

    sucess = vsSuccess && fsSuccess;
    if (sucess) {
        CZ_LOG(LogShaderCompiler, Info, "Shader: {} Compiled", rep.Name);
    } else {
        CZ_LOG(LogShaderCompiler, Error, "Failed to Compile Shader: {}",
               rep.Name);
    }

    return sucess;
}
