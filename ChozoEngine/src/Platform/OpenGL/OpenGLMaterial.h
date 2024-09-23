#pragma once

#include "Chozo/Renderer/Material.h"
#include "OpenGLShader.h"

namespace Chozo {

    class OpenGLMaterial : public Material
    {
    public:
        OpenGLMaterial(const Ref<Shader>& shader, const std::string& name);
        OpenGLMaterial(const Ref<Material>& material, const std::string& name);
        virtual ~OpenGLMaterial() override;

        virtual void Set(const std::string& name, const UniformValue& value) override;
		virtual void Set(const std::string& name, const Ref<Texture>& texture) override;

		virtual std::string GetName() override { return m_Name; }
		virtual Ref<Shader> GetShader() override { return m_Shader; }

        virtual std::map<std::string, UniformValue> GetUniforms() override { return m_Uniforms; }

        void Bind();
        void BindTextures();
        void PopulateUniforms(const Ref<OpenGLShader>& shader);
    private:
        Ref<OpenGLShader> m_Shader;
		std::string m_Name;
        std::map<std::string, UniformValue> m_Uniforms;
        std::vector<Ref<Texture>> m_TextureSlots;
        uint32_t m_TextureSlotIndex = 0;
    };
}