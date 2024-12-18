#pragma once

#include "Chozo/Renderer/Shader.h"
#include "OpenGLUniformBuffer.h"

typedef unsigned int GLenum; // TODO: remove!

namespace Chozo {

    typedef std::unordered_map<std::string, std::string> UniformTable;
    
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const ShaderSpecification& spec);
        OpenGLShader(const std::string& name, const std::string& vertexFilePath, const std::string& fragmentFilePath);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const std::string& GetName() const override { return m_Name; }
        virtual const RendererID& GetRendererID() const override { return m_RendererID; }

        virtual const std::unordered_map<std::string, std::string> GetUniformTable() const { return m_UniformTable; }
        virtual void SetUniform(const std::string& name, const UniformValue& value, const uint32_t count = 0) const override;
    public:
        void SetUniformBlockBinding(const std::string& name, const uint32_t bindingPoint) const;
    private:
        void SetUniformBool(const std::string& name, const bool value) const;
        void SetUniform1i(const std::string& name, const int value) const;
        void SetUniform1iV(const std::string &name, const int *values, const uint32_t count) const;
        void SetUniform1f(const std::string& name, const float value) const;
        void SetUniform2f(const std::string& name, const float v0, const float v1) const;
        void SetUniform3f(const std::string& name, const float v0, const float v1, const float v2) const;
        void SetUniform4f(const std::string& name, const float v0, const float v1, const float v2, const float v3) const;
        void SetUniformVec2(const std::string& name, const glm::vec2& vector) const;
        void SetUniformVec3(const std::string& name, const glm::vec3& vector) const;
        void SetUniformVec3(const std::string& name, const float vector[3]) const;
        void SetUniformVec4(const std::string& name, const glm::vec4& vector) const;
        void SetUniformMat3(const std::string& name, const glm::mat3& matrix) const;
        void SetUniformMat4(const std::string& name, const glm::mat4& matrix) const;
        void SetUniformMat4V(const std::string& name, const std::vector<glm::mat4>& array, const uint32_t count) const;
        int GetUniformLoaction(const std::string& name) const;
        std::string ReadFile(const std::string& filepath);
        std::string PreProcess(const std::string& source);
        void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
        void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
        void ConvertVulkanBinariesToOpenGL();
        void CreateProgram();
        void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
    private:
        uint32_t m_RendererID;
        std::unordered_map<GLenum, std::string> m_Filepaths;
        std::string m_Name;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;

        std::unordered_map<GLenum, std::vector<u_int32_t>> m_VulkanSPIRV;
        std::unordered_map<GLenum, std::vector<u_int32_t>> m_OpenGLSPIRV;
        std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;

        UniformTable m_UniformTable;
    };
}