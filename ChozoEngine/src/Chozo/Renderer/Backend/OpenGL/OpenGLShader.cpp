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

#include "Chozo/Renderer/Shader/ShaderCompiler.h"

namespace Chozo {

    OpenGLShader::OpenGLShader(const std::string& name, const std::vector<std::string> filePaths)
        : m_Name(name), m_Filepaths(filePaths)
    {
        Compile();
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
    //     const int maxTextureSlots = RenderCommand::GetMaxTextureSlots();

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

        for (auto&& filepath : m_Filepaths)
        {
            fs::path shaderFilepath(filepath);
            auto stage = ShaderUtils::GetShaderStageFromExtension(shaderFilepath.extension().string());
            fs::path cachePath = ShaderUtils::GetCachePathByNameAndStage(shaderFilepath.filename().stem().string(), stage);

            Utils::File::DeleteFile(cachePath);
        }
    }

    void OpenGLShader::Compile()
    {
        auto compiler = ShaderCompiler::Create();
        {
            Timer timer;
            m_RendererID = compiler->Compile(m_Filepaths);
            CZ_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
        }
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