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

		virtual std::string GetName() override { return m_Name; }
		virtual Ref<Shader> GetShader() override { return m_Shader; }

        void PopulateUniforms(const Ref<OpenGLShader>& shader);
    private:
        Ref<OpenGLShader> m_Shader;
		std::string m_Name;
    };
}