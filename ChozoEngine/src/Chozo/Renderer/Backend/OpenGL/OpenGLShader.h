#pragma once

#include "Chozo/Renderer/Shader/Shader.h"
#include "OpenGLUniformBuffer.h"

typedef unsigned int GLenum; // TODO: remove!

namespace Chozo {

    class OpenGLShader final : public Shader
    {
    public:
        OpenGLShader(std::string  name, const std::vector<std::string>& filePaths);
        ~OpenGLShader() override;

        void Bind() const override;
        void Unbind() const override;

        const std::string& GetName() const override { return m_Name; }
        const RendererID& GetRendererID() const override { return m_RendererID; }
        ShaderReflection GetReflection() const override { return m_Reflection; }

        void SetUniform(const std::string& name, const UniformValue& value, uint32_t count) const override;
        void SetUniformBlockBinding(const std::string& name, uint32_t bindingPoint) const;
        void ClearCache() override;
        void Compile() override;
        void AsyncCompile() override;
    private:
        void SetUniformBool(const std::string& name, bool value) const;
        void SetUniform1i(const std::string& name, int value) const;
        void SetUniform1iV(const std::string &name, const int *values, uint32_t count) const;
        void SetUniform1f(const std::string& name, float value) const;
        void SetUniform2f(const std::string& name, float v0, float v1) const;
        void SetUniform2f(const std::string& name, const std::array<float, 2>& value) const;
        void SetUniform2f(const std::string& name, const glm::vec2& value) const;
        void SetUniform3f(const std::string& name, float v0, float v1, float v2) const;
        void SetUniform3f(const std::string& name, const std::array<float, 3>& value) const;
        void SetUniform3f(const std::string& name, const glm::vec3& value) const;
        void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
        void SetUniform4f(const std::string& name, const std::array<float, 4>& value) const;
        void SetUniform4f(const std::string& name, const glm::vec4& value) const;
        void SetUniform3m(const std::string& name, const glm::mat3& matrix) const;
        void SetUniform4m(const std::string& name, const glm::mat4& matrix) const;
        void SetUniform4mv(const std::string& name, const std::vector<glm::mat4>& array, uint32_t count) const;
        int GetUniformLocation(const std::string& name) const;
    private:
        uint32_t m_RendererID{};
        std::vector<std::string> m_FilePaths;
        std::string m_Name;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;
    };
}