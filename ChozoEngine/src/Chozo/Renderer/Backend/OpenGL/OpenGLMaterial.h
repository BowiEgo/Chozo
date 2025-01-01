#pragma once

#include "Chozo/Renderer/Material.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"

namespace Chozo {

    class OpenGLMaterial : public Material
    {
    public:
        OpenGLMaterial(const Ref<Shader>& shader, std::string name);
        OpenGLMaterial(const Ref<Material>& material, std::string name);
        ~OpenGLMaterial() override = default;

		void CopyProperties(Ref<Material> other) override;

        void Set(const std::string& name, const UniformValue& value) override;
		void Set(const std::string& name, const Ref<Texture>& texture) override;
		void SetTextureHandle(const std::string& name, AssetHandle handle) override;
		void Set(const std::string& name, const Ref<Texture2D>& texture) override;
		void Set(const std::string& name, const Ref<TextureCube>& texture) override;

		std::string GetName() override { return m_Name; }
		Ref<Shader> GetShader() const override { return m_Shader; }

        std::map<std::string, UniformValue> GetUniforms() override { return m_Uniforms; }
        Ref<Texture2D> GetTexture(std::string name) override;

        std::vector<Ref<Texture>> GetAllTextures() const override { return m_TextureSlots; }
        uint32_t GetLastTextureSlotIndex() const override { return m_TextureSlotIndex; }

        void Bind();
        void BindTextures();
        void PopulateUniforms(const Ref<OpenGLShader>& shader);
    private:
        Ref<OpenGLShader> m_Shader;
		std::string m_Name;
        std::map<std::string, UniformValue> m_Uniforms;
        uint32_t m_TextureSlotIndex = 0;
    };
}