#include "OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Chozo/Core/Timer.h"
#include "Chozo/Renderer/RenderCommand.h"
#include "Chozo/FileSystem/FileStream.h"

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

        static void PrintSPIRReSource(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, std::unordered_map<std::string, std::string>& uniformTable)
        {
            const auto& bufferType = compiler.get_type(resource.base_type_id);
            std::string bufferName = compiler.get_name(resource.id);
            uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            int memberCount = bufferType.member_types.size();

            // CZ_CORE_TRACE("  Name = {0}", resource.name);
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
                std::string SPIRType = Utils::GetSPIRType(memberType);

                // CZ_CORE_TRACE("    Member: {0}", memberName);
                // CZ_CORE_TRACE("    Size: {0}", memberSize);
                // CZ_CORE_TRACE("    Offset: {0}", memberOffset);
                // CZ_CORE_TRACE("    Type: {0}", SPIRType);

                uniformTable[resource.name + "." + memberName] = SPIRType;
            }
        }
    }

    OpenGLShader::OpenGLShader(const ShaderSpecification &spec)
    {
        Utils::File::CreateDirectoryIfNeeded(Utils::File::GetShaderCacheDirectory());

        std::unordered_map<GLenum, std::string> shaderSources;
        std::string vertexSrc = ReadFile(spec.VertexFilepath);
        std::string fragmentSrc = ReadFile(spec.FragmentFilepath);
        fragmentSrc = PreProcess(fragmentSrc);
        shaderSources[GL_VERTEX_SHADER] = vertexSrc;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
        // Compile(shaderSources);

        fs::path vertexPath(spec.VertexFilepath);
        m_Filepaths[GL_VERTEX_SHADER] = vertexPath;
        fs::path fragmentPath(spec.FragmentFilepath);
        m_Filepaths[GL_FRAGMENT_SHADER] = fragmentPath;

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

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexFilePath, const std::string& fragmentFilePath)
        : m_Name(name)
    {
        Utils::File::CreateDirectoryIfNeeded(Utils::File::GetShaderCacheDirectory());

        std::unordered_map<GLenum, std::string> shaderSources;
        std::string vertexSrc = ReadFile(vertexFilePath);
        std::string fragmentSrc = ReadFile(fragmentFilePath);
        fragmentSrc = PreProcess(fragmentSrc);
        shaderSources[GL_VERTEX_SHADER] = vertexSrc;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
        // Compile(shaderSources);

        fs::path vertexPath(vertexFilePath);
        m_Filepaths[GL_VERTEX_SHADER] = vertexPath;
        fs::path fragmentPath(fragmentFilePath);
        m_Filepaths[GL_FRAGMENT_SHADER] = fragmentPath;

        {
            Timer timer;
            CompileOrGetVulkanBinaries(shaderSources);
            ConvertVulkanBinariesToOpenGL();
            // CreateProgram();
            Compile(m_OpenGLSourceCode);
            CZ_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
        }

        // GLuint uniformBlockIndex0 = glGetUniformBlockIndex(m_RendererID, "Camera");
        // GLuint uniformBlockIndex1 = glGetUniformBlockIndex(m_RendererID, "SceneData");
        // glUniformBlockBinding(m_RendererID, uniformBlockIndex0, 0);
        // glUniformBlockBinding(m_RendererID, uniformBlockIndex1, 1);
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

         // Max texture slots count is different by platforms, so we need to preprocess the fragment source.
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
            // CZ_CORE_WARN("{0}", modifiedSource);
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
        // CZ_CORE_WARN("{0}", modifiedSource);

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
            // CZ_CORE_WARN("Compile Source: {0}", kv.second);
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
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        const bool optimize = false;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        
        fs::path cacheDirectory = Utils::File::GetShaderCacheDirectory();

        auto& shaderData = m_VulkanSPIRV;
        shaderData.clear();
        for (auto&& [stage, source] : shaderSources)
        {
            fs::path shaderFilepath = m_Filepaths[stage];
            fs::path cachePath = cacheDirectory / (shaderFilepath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

            // TODO: Change to FileReader
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
                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_Filepaths[stage].c_str(), options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    CZ_CORE_ERROR(module.GetErrorMessage());
                    CZ_CORE_ASSERT(false, "");
                }

                shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

                FileStreamWriter writer(cachePath);
                if (writer.IsStreamGood())
                {
                    auto& data = shaderData[stage];
		            writer.WriteData((char*)shaderData[stage].data(), data.size() * sizeof(uint32_t));
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
        
        fs::path cacheDirectory = Utils::File::GetShaderCacheDirectory();

        shaderData.clear();
        m_OpenGLSourceCode.clear();
        for (auto&& [stage, spirv] : m_VulkanSPIRV)
        {
            fs::path shaderFilepath = m_Filepaths[stage];
            fs::path cachePath = cacheDirectory / (shaderFilepath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

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
            CZ_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_Filepaths[GL_VERTEX_SHADER], infoLog.data());

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

        // CZ_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_Filepaths[stage]);
        // CZ_CORE_TRACE("   {0} plain uniforms", resources.gl_plain_uniforms.size());
        // CZ_CORE_TRACE("   {0} push constant", resources.push_constant_buffers.size());
        // CZ_CORE_TRACE("   {0} uniform buffers", resources.uniform_buffers.size());
        // CZ_CORE_TRACE("   {0} sampled images", resources.sampled_images.size());
        // CZ_CORE_TRACE("   {0} storage buffers", resources.storage_buffers.size());
        // CZ_CORE_TRACE("   {0} separate samplers", resources.separate_samplers.size());
        // CZ_CORE_TRACE("   {0} separate images", resources.separate_images.size());

        // Push Constants
        // CZ_CORE_TRACE("Push contants:");
        for (const auto resource : resources.push_constant_buffers)
            Utils::PrintSPIRReSource(compiler, resource, m_UniformTable);

        // Uniform Buffers
        // CZ_CORE_TRACE("Uniform buffers:");
        for (const auto resource : resources.uniform_buffers)
            // Utils::PrintSPIRReSource(compiler, resource, m_UniformTable);

        // Sampled Images (used for textures and samplers)
        // CZ_CORE_TRACE("Sampled images:");
        for (const auto resource : resources.sampled_images)
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

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::SetUniform(const std::string &name, const UniformValue &value, const uint32_t count) const
    {
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, bool>) {
                SetUniformBool(name, val);
            } else if constexpr (std::is_same_v<T, int>) {
                SetUniform1i(name, val);
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                SetUniform1i(name, val);
            } else if constexpr (std::is_same_v<T, float>) {
                SetUniform1f(name, val);
            } else if constexpr (std::is_same_v<T, std::pair<float, float>>) {
                SetUniform2f(name, val.first, val.second);
            } else if constexpr (std::is_same_v<T, std::tuple<float, float, float>>) {
                SetUniform3f(name, std::get<0>(val), std::get<1>(val), std::get<2>(val));
            } else if constexpr (std::is_same_v<T, std::tuple<float, float, float, float>>) {
                SetUniform4f(name, std::get<0>(val), std::get<1>(val), std::get<2>(val), std::get<3>(val));
            } else if constexpr (std::is_same_v<T, glm::vec2>) {
                SetUniformVec2(name, val);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
                SetUniformVec3(name, val);
            } else if constexpr (std::is_same_v<T, float[3]>) {
                SetUniformVec3(name, val);
            } else if constexpr (std::is_same_v<T, glm::vec4>) {
                SetUniformVec4(name, val);
            } else if constexpr (std::is_same_v<T, glm::mat3>) {
                SetUniformMat3(name, val);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
                SetUniformMat4(name, val);
            } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                SetUniform1iV(name, val.data(), count);
            } else if constexpr (std::is_same_v<T, std::vector<glm::mat4>>) {
                SetUniformMat4V(name, val, count);
            }
        }, value);
    }

    void OpenGLShader::SetUniformBlockBinding(const std::string& name, const uint32_t bindingPoint) const
    {
        GLuint uniformBlockIndex = glGetUniformBlockIndex(m_RendererID, name.c_str());
        if (uniformBlockIndex != GL_INVALID_INDEX)
            glUniformBlockBinding(m_RendererID, uniformBlockIndex, bindingPoint);
        else
            CZ_CORE_WARN("Uniform Block '{}' not found in shader!", name);
    }

    void OpenGLShader::ClearCache()
    {
        fs::path cacheDirectory = Utils::File::GetShaderCacheDirectory();

        for (auto&& [stage, filepath] : m_Filepaths)
        {
            fs::path shaderFilepath(filepath);
            fs::path cachePath = cacheDirectory / (shaderFilepath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

            Utils::File::DeleteFile(cachePath);
        }
    }

    void OpenGLShader::Recompile()
    {
        Timer timer;

        std::unordered_map<GLenum, std::string> shaderSources;
        std::string vertexSrc = ReadFile(m_Filepaths[GL_VERTEX_SHADER]);
        std::string fragmentSrc = ReadFile(m_Filepaths[GL_FRAGMENT_SHADER]);
        fragmentSrc = PreProcess(fragmentSrc);
        shaderSources[GL_VERTEX_SHADER] = vertexSrc;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;

        CompileOrGetVulkanBinaries(shaderSources);
        ConvertVulkanBinariesToOpenGL();
        Compile(m_OpenGLSourceCode);
        CZ_CORE_WARN("Shader recompile took {0} ms", timer.ElapsedMillis());
    }

    void OpenGLShader::SetUniformBool(const std::string &name, const bool value) const
    {
        glUniform1i(GetUniformLoaction(name), value ? 1 : 0);
    }

    void OpenGLShader::SetUniform1i(const std::string &name, const int value) const
    {
        glUniform1i(GetUniformLoaction(name), value);
    }

    void OpenGLShader::SetUniform1iV(const std::string &name, const int *values, const uint32_t count) const
    {
        glUniform1iv(GetUniformLoaction(name), count, values);
    }

    void OpenGLShader::SetUniform1f(const std::string& name, const float value) const
    {
        glUniform1f(GetUniformLoaction(name), value);
    }

    void OpenGLShader::SetUniform2f(const std::string& name, const float v0, const float v1) const
    {
        glUniform2f(GetUniformLoaction(name), v0, v1);
    }

    void OpenGLShader::SetUniform3f(const std::string& name, const float v0, const float v1, const float v2) const
    {
        glUniform3f(GetUniformLoaction(name), v0, v1, v2);
    }

    void OpenGLShader::SetUniform4f(const std::string& name, const float v0, const float v1, const float v2, const float v3) const
    {
        glUniform4f(GetUniformLoaction(name), v0, v1, v2, v3);
    }

    void OpenGLShader::SetUniformVec2(const std::string &name, const glm::vec2 &vector) const
    {
        glUniform2f(GetUniformLoaction(name), vector.x, vector.y);
    }

    void OpenGLShader::SetUniformVec3(const std::string &name, const glm::vec3 &vector) const
    {
        glUniform3f(GetUniformLoaction(name), vector.x, vector.y, vector.z);
    }

    void OpenGLShader::SetUniformVec3(const std::string &name, const float vector[3]) const
    {
        glUniform3f(GetUniformLoaction(name), vector[0], vector[1], vector[2]);
    }

    void OpenGLShader::SetUniformVec4(const std::string &name, const glm::vec4 &vector) const
    {
        glUniform4f(GetUniformLoaction(name), vector.x, vector.y, vector.z, vector.w);
    }

    void OpenGLShader::SetUniformMat3(const std::string& name, const glm::mat3& matrix) const
    {
        glUniformMatrix3fv(GetUniformLoaction(name), 1, GL_FALSE, &matrix[0][0]);
    }

    void OpenGLShader::SetUniformMat4(const std::string& name, const glm::mat4& matrix) const
    {
        glUniformMatrix4fv(GetUniformLoaction(name), 1, GL_FALSE, &matrix[0][0]);
    }

    void OpenGLShader::SetUniformMat4V(const std::string& name, const std::vector<glm::mat4>& value, const uint32_t count) const
    {
        glUniformMatrix4fv(GetUniformLoaction(name), count, GL_FALSE, &value[0][0][0]);
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