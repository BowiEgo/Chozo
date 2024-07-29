#include "OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Chozo/Core/Timer.h"
#include "Chozo/Renderer/RenderCommand.h"

namespace Chozo {

    namespace Utils {

        static GLenum ShaderTypeFromString(const std::string& type)
        {
            if (type == "vertex")
                return GL_VERTEX_SHADER;
            if (type == "fragment" || type == "pixel")
                return GL_FRAGMENT_SHADER;

            CZ_CORE_ASSERT(false, "Unknown shader type!");
            return 0;
        }

        static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
                case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
            }
            CZ_CORE_ASSERT(false, "");
            return (shaderc_shader_kind)0;
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

        static const char* GetCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "../assets/cache/shader/opengl";
        }

        static void CreateCacheDirectoryIfNeeded()
        {
            {
                std::string cacheDirectory = GetCacheDirectory();
                if (!std::filesystem::exists(cacheDirectory))
                    std::filesystem::create_directories(cacheDirectory);
            }
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
    }

    OpenGLShader::OpenGLShader(const ShaderSpecification &spec)
    {
        Utils::CreateCacheDirectoryIfNeeded();

        std::unordered_map<GLenum, std::string> shaderSources;
        std::string vertexSrc = ReadFile(spec.VertexFilepath);
        std::string fragmentSrc = ReadFile(spec.FragmentFilepath);
        fragmentSrc = PreProcess(fragmentSrc);
        shaderSources[GL_VERTEX_SHADER] = vertexSrc;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
        // Compile(shaderSources);

        std::filesystem::path path(spec.VertexFilepath);
        m_Filepath = path.parent_path();

        {
            Timer timer;
            CompileOrGetVulkanBinaries(shaderSources);
            ConvertVulkanBinariesToOpenGL();
            // CreateProgram();
            Compile(m_OpenGLSourceCode);
            CZ_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
        }

        // Extract name from filepath
        auto lastSlash = spec.VertexFilepath.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot = spec.VertexFilepath.rfind('.');
        auto count = lastDot == std::string::npos ? spec.VertexFilepath.size() - lastSlash : lastDot - lastSlash;
        m_Name = spec.VertexFilepath.substr(lastSlash, count);
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string &vertexSrc, const std::string &fragmentSrc)
        : m_Name(name)
    {
        std::unordered_map<GLenum, std::string> shaderSources;
        shaderSources[GL_VERTEX_SHADER] = vertexSrc;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
        Compile(shaderSources);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    std::string OpenGLShader::ReadFile(const std::string &filepath)
    {
        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
        }
        else
        {
            CZ_CORE_ASSERT("Could not open file '{0}'", filepath);
        }

        return result;
    }

    std::string OpenGLShader::PreProcess(const std::string &source)
    {
        std::string modifiedSource = source;
        const std::string defineToken = "#define";
        const std::string typeToken = "MAX_TEXTURE_SLOTS";
        const std::string caseToken = "// case";
        const int maxTextureSlots = RenderCommand::GetMaxTextureSlots();

        size_t pos = modifiedSource.find(defineToken, 0);
        while (pos != std::string::npos)
        {
            size_t eol = modifiedSource.find_first_of("\r\n", pos);
            modifiedSource.replace(pos, eol - pos, "");
            pos = modifiedSource.find(defineToken, 0);
        }
        
        pos = modifiedSource.find(typeToken, 0);
        while (pos != std::string::npos)
        {
            modifiedSource.replace(pos, typeToken.length(), std::to_string(maxTextureSlots));
            CZ_CORE_WARN("{0}", modifiedSource);
            pos = modifiedSource.find(typeToken, pos + std::to_string(maxTextureSlots).length());
        }

        pos = modifiedSource.find(caseToken, 0);
        int index = 0;
        while (pos != std::string::npos && index < maxTextureSlots)
        {
            modifiedSource.replace(pos, caseToken.length(), "case");
            pos = modifiedSource.find(caseToken, pos);
            index++;
        }
        CZ_CORE_WARN("{0}", modifiedSource);

        return modifiedSource;
    }

    void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string> &shaderSources)
    {
        GLuint program = glCreateProgram();
        CZ_CORE_ASSERT(shaderSources.size() <= 2, "Only support 2 shaders for now");
        std::array<GLenum, 2> glShaderIDs;
        int glShaderIDIndex = 0;

        for (auto& kv : shaderSources)
        {
            GLenum type = kv.first;
            const std::string& source = kv.second;

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
                return;
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
            return;
        }

        // Always detach shaders after a successful link.
        for (auto id : glShaderIDs)
            glDetachShader(program, id);

        m_RendererID = program;
    }

    void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string> &shaderSources)
    {
        GLuint program = glCreateProgram();

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        const bool optimize = false;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        
        std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

        auto& shaderData = m_VulkanSPIRV;
        shaderData.clear();
        for (auto&& [stage, source] : shaderSources)
        {
            std::filesystem::path shaderFilepath = m_Filepath;
            std::filesystem::path cachePath = cacheDirectory / (shaderFilepath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

            std::ifstream in(cachePath, std::ios::in | std::ios::binary);
            if (in.is_open())
            {
                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                in.seekg(0, std::ios::beg);

                auto& data = shaderData[stage];
                data.resize(size / sizeof(uint32_t));
                in.read((char*)data.data(), size);
            }
            else
            {
                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    CZ_CORE_ERROR(module.GetErrorMessage());
                    CZ_CORE_ASSERT(false, "");
                }

                shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

                std::ofstream out(cachePath, std::ios::out | std::ios::binary);
                if (out.is_open())
                {
                    auto& data = shaderData[stage];
                    out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                    out.flush();
                    out.close(); 
                }
            }
        }

        for (auto&& [stage, data] : shaderData)
            Reflect(stage, data);
    }

    void OpenGLShader::ConvertVulkanBinariesToOpenGL()
    {
        auto& shaderData = m_OpenGLSPIRV;

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        const bool optimize = false;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        
        std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

        shaderData.clear();
        m_OpenGLSourceCode.clear();
        for (auto&& [stage, spirv] : m_VulkanSPIRV)
        {
            std::filesystem::path shaderFilepath = m_Filepath;
            std::filesystem::path cachePath = cacheDirectory / (shaderFilepath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

            std::ifstream in(cachePath, std::ios::in | std::ios::binary);
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

    void OpenGLShader::CreateProgram()
    {
        GLuint program = glCreateProgram();

        std::vector<GLuint> shaderIDs;
        for (auto&& [stage,  spirv] : m_OpenGLSPIRV)
        {
            GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
            glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
            glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
            glAttachShader(program, shaderID);
        }

        glLinkProgram(program);

        GLint isLinked;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

        if (isLinked == GL_FALSE)
        {
            GLint maxLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
            CZ_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_Filepath, infoLog.data());

            glDeleteProgram(program);

            for (auto id : shaderIDs)
                glDeleteShader(id);
        }

        for (auto id : shaderIDs)
        {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        m_RendererID = program;
    }

    void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t> &shaderData)
    {
        spirv_cross::Compiler compiler(shaderData);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        CZ_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_Filepath);
        CZ_CORE_TRACE("   {0} uinform buffers", resources.uniform_buffers.size());
        CZ_CORE_TRACE("   {0} resources", resources.sampled_images.size());

        CZ_CORE_TRACE("Uniform buffers:");
        for (const auto& resources : resources.uniform_buffers)
        {
            const auto& bufferType = compiler.get_type(resources.base_type_id);
            std::string bufferName = compiler.get_member_name(resources.base_type_id, 0);
            uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
            uint32_t binding = compiler.get_decoration(resources.id, spv::DecorationBinding);
            int memberCount = bufferType.member_types.size();

            CZ_CORE_TRACE("  {0}", resources.name);
            CZ_CORE_TRACE("  Name = {0}", bufferName);
            CZ_CORE_TRACE("  Size = {0}", bufferSize);
            CZ_CORE_TRACE("  Binding = {0}", binding);
            CZ_CORE_TRACE("  Members = {0}", memberCount);
        }
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::UploadUniformInt(const std::string &name, int value)
    {
        glUniform1i(GetUniformLoaction(name), value);
    }

    void OpenGLShader::UploadUniformIntArray(const std::string &name, int *values, uint32_t count)
    {
        glUniform1iv(GetUniformLoaction(name), count, values);
    }

    void OpenGLShader::UploadUniformFloat(const std::string &name, float value)
    {
        glUniform1f(GetUniformLoaction(name), value);
    }

    void OpenGLShader::UploadUniformFloat2(const std::string &name, const glm::vec2 &value)
    {
        glUniform2f(GetUniformLoaction(name), value.x, value.y);
    }

    void OpenGLShader::UploadUniformFloat3(const std::string &name, const glm::vec3 &value)
    {
        glUniform3f(GetUniformLoaction(name), value.x, value.y, value.z);
    }

    void OpenGLShader::UploadUniformFloat4(const std::string &name, const glm::vec4 &value)
    {
        glUniform4f(GetUniformLoaction(name), value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::UploadUniformMat3(const std::string &name, const glm::mat3 &matrix)
    {
        glUniformMatrix3fv(GetUniformLoaction(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadUniformMat4(const std::string &name, const glm::mat4 &matrix)
    {
        glUniformMatrix4fv(GetUniformLoaction(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    int OpenGLShader::GetUniformLoaction(const std::string& name) const
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];

        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
            std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;

        m_UniformLocationCache[name] = location;
        return location;
    }
}