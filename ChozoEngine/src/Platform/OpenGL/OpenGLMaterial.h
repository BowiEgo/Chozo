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

        virtual void Set(const std::string& name, const std::string& sourcePath) override;
        virtual void Set(const std::string& name, const UniformValue& value) override;
		virtual void Set(const std::string& name, const Ref<Texture>& texture) override;
		virtual void Set(const std::string& name, const Ref<Texture2D>& texture) override;
		virtual void Set(const std::string& name, const Ref<TextureCube>& texture) override;

		virtual std::string GetName() override { return m_Name; }
		virtual Ref<Shader> GetShader() const override { return m_Shader; }

        virtual std::map<std::string, UniformValue> GetUniforms() override { return m_Uniforms; }
        virtual inline std::string GetUniromSourcePath(const std::string& name) override { return m_UniformSourcePaths[name]; }

        void Bind();
        void BindTextures();
        void PopulateUniforms(const Ref<OpenGLShader>& shader);
    private:
        Ref<OpenGLShader> m_Shader;
		std::string m_Name;
        std::map<std::string, UniformValue> m_Uniforms;
        std::unordered_map<std::string, std::string> m_UniformSourcePaths;
        std::vector<Ref<Texture>> m_TextureSlots;
        uint32_t m_TextureSlotIndex = 0;
    };
}