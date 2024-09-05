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
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");
        virtual ~Material() = default;

        void Set(const std::string& name, const UniformValue& value);
		void Set(const std::string& name, const Ref<Texture2D>& texture);
		void Set(const std::string& name, const Ref<TextureCube>& texture);

        std::unordered_map<std::string, UniformValue> GetUniforms() { return m_Uniforms; }

		virtual std::string GetName() = 0;
		virtual Ref<Shader> GetShader() = 0;
    protected:
        std::unordered_map<std::string, UniformValue> m_Uniforms;
    };
}
