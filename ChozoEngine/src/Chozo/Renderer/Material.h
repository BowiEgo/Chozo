#pragma once

#include "czpch.h"
#include "Shader.h"
#include "Texture.h"

namespace Chozo
{

    class Material
    {
    public:
        static Ref<Material> Create(const std::string& name = "");
        static Ref<Material> Create(Ref<Shader> shader, const std::string& name = "");
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");
        virtual ~Material() = default;

        virtual void Set(const std::string& name, const std::string& sourcePath) = 0;
        virtual void Set(const std::string& name, const UniformValue& value) = 0;
		virtual void Set(const std::string& name, const Ref<Texture>& texture) = 0;

        virtual std::map<std::string, UniformValue> GetUniforms() = 0;
        virtual std::string GetUniromSourcePath(const std::string& name) = 0;

		virtual std::string GetName() = 0;
		virtual Ref<Shader> GetShader() = 0;
    };
}
