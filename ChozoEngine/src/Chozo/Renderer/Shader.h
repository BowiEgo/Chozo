#pragma once

#include "RendererTypes.h"

#include "czpch.h"
#include <glm/glm.hpp>

namespace Chozo {

    using UniformValue = std::variant<
        bool,
        int,
        unsigned int,
        float,
        std::pair<float, float>,
        std::tuple<float, float, float>,
        std::tuple<float, float, float, float>,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat3,
        glm::mat4,
        std::vector<glm::mat4>,
        std::vector<int>
    >;

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
        virtual const RendererID& GetRendererID() const = 0;

        virtual void SetUniform(const std::string& name, const UniformValue& value, const uint32_t count = 0) = 0;
        virtual void SetUniformBlockBinding() const = 0;
        virtual void SetUniformBlockBinding(const std::string& name, const uint32_t bindingPoint) const = 0;

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