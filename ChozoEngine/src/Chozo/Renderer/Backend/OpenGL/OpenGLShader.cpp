#include "OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <utility>

#include "Chozo/Renderer/RenderCommand.h"
#include "Chozo/FileSystem/FileStream.h"

#include "Chozo/Renderer/Shader/ShaderCompiler.h"

#include "Chozo/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Chozo {

    OpenGLShader::OpenGLShader(std::string name, const std::vector<std::string>& filePaths)
        : m_FilePaths(filePaths), m_Name(std::move(name))
    {
        OpenGLShader::Compile();
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    // std::string OpenGLShader::PreProcess(const std::string &source)
    // {
    //     std::string modifiedSource = source;

    //      // Max texture slots count is different by platforms, so we need to preprocess the fragment source.
    //     const std::string defineToken = "#define";
    //     const std::string typeToken = "MAX_TEXTURE_SLOTS";
    //     const std::string caseToken = "// case";
    //     const int maxTextureSlots = RenderCommand::GetMaxTextureSlotCount();

    //     size_t pos = modifiedSource.find(defineToken, 0);
    //     while (pos != std::string::npos)
    //     {
    //         size_t eol = modifiedSource.find_first_of("\r\n", pos);
    //         modifiedSource.replace(pos, eol - pos, "");
    //         pos = modifiedSource.find(defineToken, 0);
    //     }
        
    //     pos = modifiedSource.find(typeToken, 0);
    //     while (pos != std::string::npos)
    //     {
    //         modifiedSource.replace(pos, typeToken.length(), std::to_string(maxTextureSlots));
    //         // CZ_CORE_WARN("{0}", modifiedSource);
    //         pos = modifiedSource.find(typeToken, pos + std::to_string(maxTextureSlots).length());
    //     }

    //     pos = modifiedSource.find(caseToken, 0);
    //     int index = 0;
    //     while (pos != std::string::npos && index < maxTextureSlots)
    //     {
    //         modifiedSource.replace(pos, caseToken.length(), "case");
    //         pos = modifiedSource.find(caseToken, pos);
    //         index++;
    //     }
    //     // CZ_CORE_WARN("{0}", modifiedSource);

    //     return modifiedSource;
    // }

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
        switch (Uniform::GetType(value)) {
            case UniformType::Bool: SetUniformBool(name, Uniform::As<bool>(value)); break;
            case UniformType::Int:
            case UniformType::Uint: SetUniform1i(name, Uniform::As<int>(value)); break;
            case UniformType::Float: SetUniform1f(name, Uniform::As<float>(value)); break;
            case UniformType::Float2: SetUniform2f(name, Uniform::As<std::array<float, 2>>(value)); break;
            case UniformType::Float3: SetUniform3f(name, Uniform::As<std::array<float, 3>>(value)); break;
            case UniformType::Float4: SetUniform4f(name, Uniform::As<std::array<float, 4>>(value)); break;
            case UniformType::Vec2: SetUniform2f(name, Uniform::As<glm::vec2>(value)); break;
            case UniformType::Vec3: SetUniform3f(name, Uniform::As<glm::vec3>(value)); break;
            case UniformType::Vec4: SetUniform4f(name, Uniform::As<glm::vec4>(value)); break;
            case UniformType::Mat3: SetUniform3m(name, Uniform::As<glm::mat3>(value)); break;
            case UniformType::Mat4: SetUniform4m(name, Uniform::As<glm::mat4>(value)); break;
            case UniformType::Mat4Vec: SetUniform4mv(name, Uniform::As<std::vector<glm::mat4>>(value), count); break;
            case UniformType::IntVec: SetUniform1iV(name, Uniform::As<std::vector<int>>(value).data(), count); break;
            default: return;
        }
    }

    void OpenGLShader::SetUniformBlockBinding(const std::string& name, const uint32_t bindingPoint) const
    {
        const GLuint uniformBlockIndex = glGetUniformBlockIndex(m_RendererID, name.c_str());
        if (uniformBlockIndex != GL_INVALID_INDEX)
            glUniformBlockBinding(m_RendererID, uniformBlockIndex, bindingPoint);
        else
            CZ_CORE_WARN("Uniform Block '{}' not found in shader!", name);
    }

    void OpenGLShader::ClearCache()
    {
        fs::path cacheDirectory = Utils::File::GetShaderCacheDirectory();

        for (auto&& filepath : m_FilePaths)
        {
            fs::path shaderFilepath(filepath);
            auto stage = ShaderUtils::GetShaderStageFromExtension(shaderFilepath.extension().string());
            fs::path cachePath = ShaderUtils::GetCachePathByNameAndStage(shaderFilepath.filename().stem().string(), stage);

            Utils::File::DeleteFile(cachePath);
        }
    }

    void OpenGLShader::Compile()
    {
        auto compiler = ShaderCompiler::Create(m_Name);
        m_RendererID = compiler->Compile(m_FilePaths);
        m_Reflection = compiler->Reflect();
    }

    void OpenGLShader::AsyncCompile()
    {
        auto sharedContext = Application::Get().GetWindow().GetSharedWindow();

        glfwMakeContextCurrent(sharedContext);
        Compile();
        glfwMakeContextCurrent(nullptr);
    }

    void OpenGLShader::SetUniformBool(const std::string &name, const bool value) const
    {
        glUniform1i(GetUniformLocation(name), value ? 1 : 0);
    }

    void OpenGLShader::SetUniform1i(const std::string &name, const int value) const
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void OpenGLShader::SetUniform1iV(const std::string &name, const int *values, const uint32_t count) const
    {
        glUniform1iv(GetUniformLocation(name), count, values);
    }

    void OpenGLShader::SetUniform1f(const std::string& name, const float value) const
    {
        glUniform1f(GetUniformLocation(name), value);
    }

    void OpenGLShader::SetUniform2f(const std::string& name, const float v0, const float v1) const
    {
        glUniform2f(GetUniformLocation(name), v0, v1);
    }

    void OpenGLShader::SetUniform2f(const std::string& name, const std::array<float, 2>& value) const
    {
        SetUniform2f(name, value[0], value[1]);
    }

    void OpenGLShader::SetUniform2f(const std::string& name, const glm::vec2& value) const
    {
        SetUniform2f(name, value.x, value.y);
    }

    void OpenGLShader::SetUniform3f(const std::string& name, const float v0, const float v1, const float v2) const
    {
        glUniform3f(GetUniformLocation(name), v0, v1, v2);
    }

    void OpenGLShader::SetUniform3f(const std::string& name, const std::array<float, 3>& value) const
    {
        SetUniform3f(name, value[0], value[1], value[2]);
    }

    void OpenGLShader::SetUniform3f(const std::string& name, const glm::vec3& value) const
    {
        SetUniform3f(name, value.x, value.y, value.z);
    }

    void OpenGLShader::SetUniform4f(const std::string& name, const float v0, const float v1, const float v2, const float v3) const
    {
        glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
    }

    void OpenGLShader::SetUniform4f(const std::string& name, const std::array<float, 4>& value) const
    {
        SetUniform4f(name, value[0], value[1], value[2], value[3]);
    }

    void OpenGLShader::SetUniform4f(const std::string& name, const glm::vec4& value) const
    {
        SetUniform4f(name, value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::SetUniform3m(const std::string& name, const glm::mat3& matrix) const
    {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }

    void OpenGLShader::SetUniform4m(const std::string& name, const glm::mat4& matrix) const
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }

    void OpenGLShader::SetUniform4mv(const std::string& name, const std::vector<glm::mat4>& array, const uint32_t count) const
    {
        glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, &array[0][0][0]);
    }

    int OpenGLShader::GetUniformLocation(const std::string& name) const
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];

        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
            CZ_CORE_ERROR("Uniform '{0}' doesn't exist at shader {1}!", name, m_Name);

        m_UniformLocationCache[name] = location;
        return location;
    }
}