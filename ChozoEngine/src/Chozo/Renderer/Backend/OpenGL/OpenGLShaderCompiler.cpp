#include "OpenGLShaderCompiler.h"

#include "Chozo/Renderer/Shader/GlslIncluder.h"

#include "Chozo/Core/Timer.h"
#include "Chozo/Utilities/FileUtils.h"
#include "Chozo/FileSystem/FileStream.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Chozo {

    namespace ShaderUtils {

        static GLenum ShaderStageToGLEnum(ShaderStage shaderStage)
        {
            switch (shaderStage) {
				#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) case ShaderStage::ENUM: return GL_##UPPER_ENUM##_SHADER;
				FOREACH_SHADER_STAGE(GENERATE_CASE)
				#undef GENERATE_CASE
				default: return (GLenum)0;
			}
        }
    }

    OpenGLShaderCompiler::OpenGLShaderCompiler(std::string &name)
    {
        m_Name = name;
    }

    RendererID OpenGLShaderCompiler::Compile(const std::vector<std::string> filePaths)
    {
        const Timer timer;

        CZ_CORE_TRACE("Compiling shader {0}...", m_Name);

        for (const auto& pathString : filePaths)
        {
            fs::path path(pathString);
            auto stage = ShaderUtils::GetShaderStageFromExtension(path.extension().string());
            const std::string source = Utils::File::ReadTextFile(pathString);

            m_Sources[stage] = source;
            m_Paths[stage] = path;
        }

        CompileToOrGetVulkanBinaries(m_Sources, m_Paths);
        DecompileVulkanBinaries();
        const RendererID program = CompileToProgram();

        CZ_CORE_TRACE("Shader compiler took {0} ms", timer.ElapsedMillis());

        return program;
    }

    void OpenGLShaderCompiler::Release()
    {
        m_OpenGLSPIRV.clear();
        m_OpenGLSourceCode.clear();
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
        
        for (auto&& [stage, spirv] : m_VulkanSpirV)
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
            // if (m_Name == "Texture")
            //     CZ_CORE_WARN("Compile Source: {0}", source);

            GLenum type = ShaderUtils::ShaderStageToGLEnum(stage);

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
