#include "OpenGLMaterial.h"

#include <utility>

#include "Chozo/Core/Application.h"
#include "Chozo/Renderer/Renderer.h"

namespace Chozo {

    static UniformValue GetUniformDefaultValue(const std::string& uniformType)
    {
        if (uniformType == "bool" || uniformType == "int" || uniformType == "uint")
            return false;
        if (uniformType == "float" || uniformType == "double")
            return 0.0f;
        if (uniformType == "Vec3")
            return glm::vec3(1.0f);
        if (uniformType == "Vec4")
            return glm::vec4(1.0f);
        if (uniformType == "Mat4")
            return glm::mat4(1.0f);
        if (uniformType == "Array")
            return std::vector<int>();

        CZ_CORE_ASSERT(false, "UniformType not supported!");
        return false;
    }

    OpenGLMaterial::OpenGLMaterial(const Ref<Shader> &shader, std::string name)
        : m_Shader(shader.As<OpenGLShader>()), m_Name(std::move(name))
    {
        m_TextureSlots.resize(Renderer::GetMaxTextureSlots());
        m_TextureAssetHandles.resize(Renderer::GetMaxTextureSlots());
        PopulateUniforms(m_Shader);
    }

    OpenGLMaterial::OpenGLMaterial(const Ref<Material> &material, std::string name)
        : m_Shader(material->GetShader().As<OpenGLShader>()), m_Name(std::move(name))
    {
        OpenGLMaterial::CopyProperties(material);
    }

    void OpenGLMaterial::CopyProperties(const Ref<Material> other)
    {
        CZ_CORE_ASSERT(m_Shader == other->GetShader(), "Copy material failed because shader is not same.");

        m_TextureSlots = other->GetAllTextures();
        m_TextureSlotIndex = other->GetLastTextureSlotIndex();
        m_TextureAssetHandles = other->GetTextureAssetHandles();

        auto uniforms = other.As<OpenGLMaterial>()->GetUniforms();
        for (const auto& [uniformName, uniformValue] : uniforms)
            Set(uniformName, uniformValue);
    }

    void OpenGLMaterial::Set(const std::string &name, const UniformValue &value)
    {
        m_Uniforms[name] = value;
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<Texture> &texture)
    {
        if (texture->GetAssetType() != AssetType::Texture)
            return;

        int textureIndex = -1;

        for (int i = 0; i < m_TextureSlots.size(); i++)
        {
            if (m_TextureSlots[i] == texture)
            {
                textureIndex = i;
                break;
            }
        }

        if (textureIndex == -1)
        {
            textureIndex = (int)m_TextureSlotIndex;
            m_TextureSlots[m_TextureSlotIndex] = texture;
            if (texture->GetType() == TextureType::Texture2D && Application::GetAssetManager()->IsAssetHandleValid(texture->Handle))
            {
                m_TextureAssetHandles[m_TextureSlotIndex] = { name, texture->Handle };
            }
            m_TextureSlotIndex++;
        }

        m_Uniforms[name] = textureIndex;
    }

    void OpenGLMaterial::SetTextureHandle(const std::string &name, const AssetHandle handle)
    {
        if (Application::GetAssetManager()->IsAssetHandleValid(handle))
        {
            int textureIndex = -1;
            for (int i = 0; i < m_TextureSlots.size(); i++)
            {
                if (m_TextureSlots[i] && m_TextureSlots[i]->Handle == handle)
                {
                    textureIndex = i;
                    break;
                }
            }

            if (textureIndex == -1)
            {
                textureIndex = (int)m_TextureSlotIndex;
                m_TextureSlots[m_TextureSlotIndex] = nullptr;
                m_TextureAssetHandles[m_TextureSlotIndex] = { name, handle };
                m_TextureSlotIndex++;
            }

            m_Uniforms[name] = textureIndex;
        }
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<Texture2D> &texture)
    {
        Set(name, texture.As<Texture>());
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<TextureCube> &texture)
    {
        Set(name, texture.As<Texture>());
    }

    Ref<Texture2D> OpenGLMaterial::GetTexture(std::string name)
    {
        const UniformValue value = m_Uniforms[name];
        uint32_t slotIndex = 0;

        if (std::holds_alternative<bool>(value))
        {
            if (std::get<bool>(value))
                CZ_CORE_ERROR("{} does not contain int/unsigned int value", name);

            return nullptr;
        }
        else if (std::holds_alternative<int>(value))
        {
            slotIndex = std::get<int>(value);
        }
        else if (std::holds_alternative<unsigned int>(value))
        {
            slotIndex = std::get<unsigned int>(value);
        }

        auto texture = m_TextureSlots[slotIndex];
        if (!texture)
        {
            const auto handle = std::get<1>(m_TextureAssetHandles[slotIndex]);
            texture = Application::GetAssetManager()->GetAsset(handle).As<Texture2D>();
            if (texture)
                m_TextureSlots[slotIndex] = texture;
        }

        return texture;
    }

    void OpenGLMaterial::Bind()
    {
        BindTextures();
        m_Shader->Bind();
        for (const auto&[name, value] : m_Uniforms)
            m_Shader->Shader::SetUniform(name, value);
    }

    void OpenGLMaterial::BindTextures()
    {
        for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
        {
            auto texture = m_TextureSlots[i];
            if (!texture)
            {
                auto handle = std::get<1>(m_TextureAssetHandles[i]);
                texture = Application::GetAssetManager()->GetAsset(handle).As<Texture2D>();
                if (texture)
                    m_TextureSlots[i] = texture;
            }

            switch (texture->GetType())
            {
            case TextureType::Texture2D:
                texture.As<OpenGLTexture2D>()->Bind(i);
                break;
            case TextureType::TextureCube:
                texture.As<OpenGLTextureCube>()->Bind(i);
                break;
            default:
                CZ_CORE_ERROR("Error: Unknown texture type!");
                break;
            }
        }
    }

    void OpenGLMaterial::PopulateUniforms(const Ref<OpenGLShader> &shader)
    {
        const ShaderReflection reflection = shader->GetReflection();
        for (const auto& uniform : reflection.uniforms)
            Set(uniform.fullName(), GetUniformDefaultValue(uniform.type));
    }
}