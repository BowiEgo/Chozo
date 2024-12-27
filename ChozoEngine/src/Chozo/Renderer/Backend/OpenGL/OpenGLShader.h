#pragma once

#include "Chozo/Renderer/Shader.h"
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
    public:
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
        void SetUniform3f(const std::string& name, float v0, float v1, float v2) const;
        void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
        void SetUniformVec2(const std::string& name, const glm::vec2& vector) const;
        void SetUniformVec3(const std::string& name, const glm::vec3& vector) const;
        void SetUniformVec3(const std::string& name, const float vector[3]) const;
        void SetUniformVec4(const std::string& name, const glm::vec4& vector) const;
        void SetUniformMat3(const std::string& name, const glm::mat3& matrix) const;
        void SetUniformMat4(const std::string& name, const glm::mat4& matrix) const;
        void SetUniformMat4V(const std::string& name, const std::vector<glm::mat4>& array, uint32_t count) const;
        int GetUniformLocation(const std::string& name) const;
    private:
        uint32_t m_RendererID{};
        std::vector<std::string> m_FilePaths;
        std::string m_Name;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;
    };
}