#include "ShaderCompiler.h"

#include "../RenderCommand.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLShaderCompiler.h"
#include "Chozo/FileSystem/FileStream.h"
#include "Chozo/Renderer/Shader/GlslIncluder.h"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Chozo {
    
    Ref<ShaderCompiler> ShaderCompiler::Create(std::string& name)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLShaderCompiler>::Create(name);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    namespace ShaderUtils {

        static const std::string GetSPIRType(const spirv_cross::SPIRType& type)
        {
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
            if (type.vecsize > 1 && type.columns == 1)
            {
                result = "Vec" + std::to_string(type.vecsize);
            }
            else if (type.columns > 1)
            {
                result = "Mat" + std::to_string(type.columns);
            }

            // Handle arrays
            if (!type.array.empty())
            {
                result = "Array" + std::to_string(type.array[0]);  // Print the size of the first dimension
                // If there are multiple dimensions, you can iterate through them
                for (size_t i = 1; i < type.array.size(); ++i)
                {
                    CZ_CORE_TRACE("Array dimension {0} size: {1}", i, type.array[i]);
                }
            }

            return result;
        }

        static void ReflectSPIRReSource(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, ShaderReflection& reflection)
        {
            const auto& bufferType = compiler.get_type(resource.base_type_id);
            std::string bufferName = compiler.get_name(resource.id);
            uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            uint32_t memberCount = bufferType.member_types.size();

            // CZ_CORE_TRACE("  Name = {0}", bufferName);
            // CZ_CORE_TRACE("  Size = {0}", bufferSize);
            // CZ_CORE_TRACE("  Binding = {0}", binding);
            // CZ_CORE_TRACE("  Members = {0}", memberCount);

            // Iterate over each member of the struct
            for (uint32_t i = 0; i < memberCount; i++)
            {
                std::string memberName = compiler.get_member_name(resource.base_type_id, i);  // Get the name of the member (e.g., "ModelMatrix")
                const auto& memberType = compiler.get_type(bufferType.member_types[i]);  // Get the type of the member

                // Size and offset of the member
                size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
                size_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
                std::string SPIRType = GetSPIRType(memberType);
                //
                // CZ_CORE_TRACE("    Member: {0}", memberName);
                // CZ_CORE_TRACE("    Type: {0}", SPIRType);
                // CZ_CORE_TRACE("    Size: {0}", memberSize);
                // CZ_CORE_TRACE("    Offset: {0}", memberOffset);

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
    }

    void ShaderCompiler::CompileToOrGetVulkanBinaries(ShaderSources &shaderSources, const ShaderPaths &shaderPaths)
    {
        // Set compile options
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        options.AddMacroDefinition("MAX_TEXTURE_SLOTS", std::to_string(RenderCommand::GetMaxTextureSlots()));

        const bool optimize = false;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);

        fs::path cacheDirectory = Utils::File::GetShaderCacheDirectory();

        for (auto&& [stage, source] : shaderSources)
        {
            fs::path shaderFilepath = shaderPaths.at(stage);
            fs::path cachePath = ShaderUtils::GetCachePathByNameAndStage(shaderFilepath.filename().stem().string(), stage);

            auto& data = m_VulkanSpirV[stage];

            FileStreamReader stream(cachePath);
            if (stream.IsStreamGood())
            {
                // Read vulkan shader binaries if exist.
                stream.ReadBinary(data);
            }
            else
            {
                // Compile source string to vulkan shader binaries if doesn't exist.
                PreProcess(shaderFilepath.string(), stage, source);

                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
                    source,
                    ShaderUtils::ShaderStageToShaderC(stage),
                    shaderFilepath.c_str(),
                    options);

                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    CZ_CORE_ERROR(module.GetErrorMessage());
                    CZ_CORE_ASSERT(false, "");
                }

                data = std::vector<uint32_t>(module.cbegin(), module.cend());

                FileStreamWriter writer(cachePath);
                if (writer.IsStreamGood())
                {
		            writer.WriteData((char*)data.data(), data.size() * sizeof(uint32_t));
                }
            }
        }
    }

    void ShaderCompiler::PreProcess(const std::string &shaderSourcePath, const ShaderStage &stage, std::string &shaderSource)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
		shaderc_util::FileFinder fileFinder;

        options.SetIncluder(std::make_unique<GlslIncluder>(&fileFinder));

        const auto preProcessingResult = compiler.PreprocessGlsl(
            shaderSource,
            ShaderUtils::ShaderStageToShaderC(stage),
            shaderSourcePath.c_str(),
            options
        );

        if (preProcessingResult.GetCompilationStatus() != shaderc_compilation_status_success)
            CZ_CORE_ASSERT("Renderer", fmt::format("Failed to pre-process \"{}\"'s {} shader.\nError: {}", shaderSourcePath, ShaderUtils::ShaderStageToString(stage), preProcessingResult.GetErrorMessage()));
        
        shaderSource = std::string(preProcessingResult.begin(), preProcessingResult.end());
    }

    ShaderReflection ShaderCompiler::Reflect()
    {
        ShaderReflection reflection;

        for (auto&& [stage, source] : m_VulkanSpirV)
        {
            spirv_cross::Compiler compiler(source);
            spirv_cross::ShaderResources resources = compiler.get_shader_resources();

            // CZ_CORE_TRACE("{0} {1}", ShaderUtils::ShaderStageToString(stage), m_Paths[stage].string());
            // CZ_CORE_TRACE("   {0} plain uniforms", resources.gl_plain_uniforms.size());
            // CZ_CORE_TRACE("   {0} push constant", resources.push_constant_buffers.size());
            // CZ_CORE_TRACE("   {0} uniform buffers", resources.uniform_buffers.size());
            // CZ_CORE_TRACE("   {0} sampled images", resources.sampled_images.size());
            // CZ_CORE_TRACE("   {0} storage buffers", resources.storage_buffers.size());
            // CZ_CORE_TRACE("   {0} separate samplers", resources.separate_samplers.size());
            // CZ_CORE_TRACE("   {0} separate images", resources.separate_images.size());

            // Push Constants
            // CZ_CORE_TRACE("Push contants:");
            for (const auto& resource : resources.push_constant_buffers)
                ShaderUtils::ReflectSPIRReSource(compiler, resource, reflection);

            // Uniform Buffers
            // CZ_CORE_TRACE("Uniform buffers:");
            for (const auto& resource : resources.uniform_buffers)
                // Utils::ReflectSPIRReSource(compiler, resource, m_UniformTable);

            // Sampled Images (used for textures and samplers)
            // CZ_CORE_TRACE("Sampled images:");
            for (const auto& resource : resources.sampled_images)
            {
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                // CZ_CORE_TRACE("  {0}", resource.name);
                // CZ_CORE_TRACE("  Binding = {0}", binding);
            }

            // Storage Buffers
            // CZ_CORE_TRACE("Storage buffers:");
            for (const auto resource : resources.storage_buffers)
            {
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                // CZ_CORE_TRACE("  {0}", resource.name);
                // CZ_CORE_TRACE("  Binding = {0}", binding);
            }

            // Separate Samplers
            // CZ_CORE_TRACE("Separate samplers:");
            for (const auto resource : resources.separate_samplers)
            {
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                // CZ_CORE_TRACE("  {0}", resource.name);
                // CZ_CORE_TRACE("  Binding = {0}", binding);
            }

            // Separate Images
            // CZ_CORE_TRACE("Separate images:");
            for (const auto resource : resources.separate_images)
            {
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                // CZ_CORE_TRACE("  {0}", resource.name);
                // CZ_CORE_TRACE("  Binding = {0}", binding);
            }
        }

        return reflection;
    }
} // namespace Chozo
