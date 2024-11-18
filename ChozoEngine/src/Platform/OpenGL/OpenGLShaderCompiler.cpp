#include "OpenGLShaderCompiler.h"

#include "Chozo/Renderer/Shader/GlslIncluder.h"

#include "Chozo/Utilities/FileUtils.h"
#include "Chozo/FileSystem/FileStream.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Chozo {

    namespace Utils {

        static GLenum ShaderStageToGLEnum(ShaderStage shaderStage)
        {
            switch (shaderStage) {
				#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) case ShaderStage::ENUM: return GL_##UPPER_ENUM##_SHADER;
				FOREACH_SHADER_STAGE(GENERATE_CASE)
				#undef GENERATE_CASE
				default: return (GLenum)0;
			}
        }

        static const char* GLShaderStageToString(GLenum stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
                case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
            }
            CZ_CORE_ASSERT(false, "");
            return nullptr;
        }

        static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:   return ".cached_vulkan.vert";
                case GL_FRAGMENT_SHADER: return ".cahced_vulkan.frag";
            }
            CZ_CORE_ASSERT(false, "");
            return "";
        }

        static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:   return ".cached_opengl.vert";
                case GL_FRAGMENT_SHADER: return ".cahced_opengl.frag";
            }
            CZ_CORE_ASSERT(false, "");
            return "";
        }
    }

    RendererID OpenGLShaderCompiler::Compile(const std::vector<std::string> filePaths)
    {
        ShaderSources shaderSources;
        ShaderPaths shaderPaths;

        for (auto pathString : filePaths)
        {
            fs::path path(pathString);
            auto type = ShaderUtils::GetShaderStageFromExtension(path.extension().string());
            std::string source = Utils::File::ReadTextFile(pathString);

            shaderSources[type] = source;
            shaderPaths[type] = path;
            // CZ_CORE_INFO(pathString);
            // CZ_CORE_INFO(source);
        }

        CompileToOrGetVulkanBinaries(shaderSources, shaderPaths);
        DecompileVulkanBinaries();
        RendererID program = CompileToProgram();

        return program;
    }

    void OpenGLShaderCompiler::DecompileVulkanBinaries()
    {
        // Set compile options
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        const bool optimize = false;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        
        for (auto&& [stage, spirv] : m_VulkanSPIRV)
        {
            spirv_cross::CompilerGLSL glslCompiler(spirv);
            spirv_cross::CompilerGLSL::Options glslOptions;
            glslOptions.version = 410;
            glslOptions.es = false;
            glslOptions.vulkan_semantics = false;
            glslOptions.separate_shader_objects = false;
            glslOptions.enable_420pack_extension = false;

            glslCompiler.set_common_options(glslOptions);
            m_OpenGLSourceCode[stage] = glslCompiler.compile();
        }
    }

    RendererID OpenGLShaderCompiler::CompileToProgram()
    {
        GLuint program = glCreateProgram();
        CZ_CORE_ASSERT(m_OpenGLSourceCode.size() <= 2, "Only support 2 shaders for now");
        std::array<GLenum, 2> glShaderIDs;
        int glShaderIDIndex = 0;

        for (auto&& [stage, source] : m_OpenGLSourceCode)
        {
            // CZ_CORE_WARN("Compile Source: {0}", kv.second);
            GLenum type = Utils::ShaderStageToGLEnum(stage);

            GLuint shader = glCreateShader(type);

            const GLchar *sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);
            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if(isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
                
                glDeleteShader(shader);

                CZ_CORE_ERROR("{0}", infoLog.data());
                CZ_CORE_ASSERT(false, "Shader compilation failure!");
            }
    
            glAttachShader(program, shader);
            glShaderIDs[glShaderIDIndex++] = shader;
        }

        // Link our program
        glLinkProgram(program);

        // Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
            
            // We don't need the program anymore.
            glDeleteProgram(program);
            // Don't leak shaders either.
            for (auto id : glShaderIDs)
                glDeleteShader(id);

            CZ_CORE_ERROR("{0}", infoLog.data());
            CZ_CORE_ASSERT(false, "Shader link failure!");
        }

        // Always detach shaders after a successful link.
        for (auto id : glShaderIDs)
            glDetachShader(program, id);

        return program;
    }

} // namespace Chozo
