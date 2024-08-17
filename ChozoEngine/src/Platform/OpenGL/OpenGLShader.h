#pragma once

#include "Chozo/Renderer/Shader.h"

typedef unsigned int GLenum; // TODO: remove!

namespace Chozo {
    
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const ShaderSpecification& spec);
        OpenGLShader(const std::string& name, const std::string& vertexFilePath, const std::string& fragmentFilePath);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const std::string& GetName() const override { return m_Name; };

        virtual void UploadUniformInt(const std::string& name, int value) override;
        virtual void UploadUniformIntArray(const std::string& name, int* values, uint32_t count) override;

        virtual void UploadUniformFloat(const std::string& name, float value) override;
        virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) override;
        virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) override;
        virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) override;

        virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
        virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
    private:
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
    };
}