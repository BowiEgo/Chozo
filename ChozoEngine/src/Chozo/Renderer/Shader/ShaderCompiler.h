#pragma once

#include "Chozo/Renderer/RendererTypes.h"
#include "Chozo/Utilities/StringUtils.h"

#include "czpch.h"
#include <shaderc/shaderc.hpp>

namespace Chozo {

    #define FOREACH_SHADER_STAGE(TYPE) \
        	TYPE(Vertex, vertex, VERTEX, vert) \
        	TYPE(Fragment, fragment, FRAGMENT, frag) \

	enum class ShaderStage : uint16_t {
		#define GENERATE_ENUM(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) ENUM,
		FOREACH_SHADER_STAGE(GENERATE_ENUM)
		#undef GENERATE_ENUM
		None
	};

    inline static std::unordered_map<std::string, ShaderStage> s_ShaderExtensionMap =
	{
		// Vertex
		{ ".vert", ShaderStage::Vertex },

        // Fragment
		{ ".frag", ShaderStage::Fragment },
		{ ".pixel", ShaderStage::Fragment },
	};

    namespace ShaderUtils {

		inline ShaderStage StringToShaderStage(std::string_view shaderStage) {
			#define GENERATE_IF(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) if (shaderStage == #ENUM) return ShaderStage::ENUM;
			FOREACH_SHADER_STAGE(GENERATE_IF)
			#undef GENERATE_IF
			return ShaderStage::None;
		}

		inline const char* ShaderStageToString(ShaderStage shaderStage) {
			switch (shaderStage) {
				#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) case ShaderStage::ENUM: return #ENUM;
				FOREACH_SHADER_STAGE(GENERATE_CASE)
				#undef GENERATE_CASE
				default: return "Unknown";
			}
		}

		inline ShaderStage GetShaderStageFromExtension(const std::string& extension)
		{
			std::string ext = Utils::String::ToLowerCopy(extension);
			if (s_ShaderExtensionMap.find(ext) == s_ShaderExtensionMap.end())
				return ShaderStage::None;

			return s_ShaderExtensionMap.at(ext);
		}

        inline shaderc_shader_kind ShaderStageToShaderC(ShaderStage shaderStage)
        {
            switch (shaderStage) {
				#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) case ShaderStage::ENUM: return shaderc_glsl_##LOWER_ENUM##_shader;
				FOREACH_SHADER_STAGE(GENERATE_CASE)
				#undef GENERATE_CASE
				default: return static_cast<shaderc_shader_kind>(0);
			}
        }

        inline std::string ShaderStageToVulkanCacheFileExtension(const ShaderStage shaderStage)
        {
            switch (shaderStage) {
                #define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) case ShaderStage::ENUM: return ".cache_vulkan." + std::string(#SHORT_ENUM);
				FOREACH_SHADER_STAGE(GENERATE_CASE)
				#undef GENERATE_CASE
				default: return "";
			}
        }

        inline fs::path GetCachePathByNameAndStage(const std::string& name, const ShaderStage& stage)
        {
            fs::path cacheDirectory = Utils::File::GetShaderCacheDirectory();
            Utils::File::CreateDirectoryIfNeeded(cacheDirectory);

            return cacheDirectory / (name + ShaderUtils::ShaderStageToVulkanCacheFileExtension(stage));
        }
	}

    using ShaderSources = std::unordered_map<ShaderStage, std::string>;
    using ShaderPaths = std::unordered_map<ShaderStage, fs::path>;
    using ShaderBinaries = std::unordered_map<ShaderStage, std::vector<u_int32_t>>;
    
    class ShaderCompiler : public RefCounted
    {
    public:
        virtual RendererID Compile(std::vector<std::string> filePaths) = 0;
        virtual void Release() = 0;

        void CompileToOrGetVulkanBinaries(ShaderSources& shaderSources, const ShaderPaths& shaderPaths);
        static void PreProcess(const std::string& shaderSourcePath, const ShaderStage& stage, std::string& shaderSource);
        void Reflect(const ShaderStage& stage, const std::vector<uint32_t>& shaderData);

        static Ref<ShaderCompiler> Create(std::string& name);
    protected:
        std::string m_Name;
        ShaderBinaries m_VulkanSpirV;
    };
} // namespace Chozo
