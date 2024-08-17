#pragma once

#include "czpch.h"
#include <glm/glm.hpp>

namespace Chozo {

    struct ShaderSpecification
    {
        std::string VertexFilepath, FragmentFilepath;
    };
    
    class Shader
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const std::string& GetName() const = 0;

        virtual void UploadUniformInt(const std::string& name, int value) = 0;
        virtual void UploadUniformIntArray(const std::string& name, int* values, uint32_t count) = 0;

        virtual void UploadUniformFloat(const std::string& name, float value) = 0;
        virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) = 0;
        virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) = 0;
        virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) = 0;

        virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
        virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

        static Ref<Shader> Create(const ShaderSpecification& spec = ShaderSpecification());
        static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
    };

    class ShaderLibrary
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() {};

		void Load(std::string_view name, const std::string& vertexSrc, const std::string& fragmentSrc);

		const Ref<Shader>& Get(const std::string& name) const;

        static Ref<ShaderLibrary> Create();
    private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };
}