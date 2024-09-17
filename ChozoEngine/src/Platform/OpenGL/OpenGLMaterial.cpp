#include "OpenGLMaterial.h"

#include "Chozo/Renderer/Renderer.h"

namespace Chozo {

    static UniformValue GetUniformDefaultValue(std::string uniformType)
    {
        if (uniformType == "bool")
            return false;
        else if (uniformType == "int" || uniformType == "uint")
            return 0;
        else if (uniformType == "float" || uniformType == "double")
            return 0.0f;
        else if (uniformType == "Vec3")
            return glm::vec3(1.0f);
        else if (uniformType == "Vec4")
            return glm::vec4(1.0f);
        else if (uniformType == "Mat4")
            return glm::mat4(1.0f);
        else if (uniformType == "Array")
            return std::vector<int>();

        CZ_CORE_ASSERT(false, "UniformType not supported!");
        return false;
    }

    OpenGLMaterial::OpenGLMaterial(const Ref<Shader> &shader, const std::string &name)
        : m_Shader(std::dynamic_pointer_cast<OpenGLShader>(shader)), m_Name(name)
    {
        m_TextureSlots.resize(Renderer::GetMaxTextureSlots());
        PopulateUniforms(m_Shader);
    }

    OpenGLMaterial::OpenGLMaterial(const Ref<Material> &material, const std::string &name)
        : m_Shader(std::dynamic_pointer_cast<OpenGLShader>(material->GetShader())), m_Name(name)
    {
        m_TextureSlots.resize(Renderer::GetMaxTextureSlots());
        PopulateUniforms(m_Shader);
    }

    OpenGLMaterial::~OpenGLMaterial()
    {
    }

    void OpenGLMaterial::Set(const std::string &name, const UniformValue &value)
    {
        m_Uniforms[name] = value;
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<Texture> &texture)
    {
        int textureIndex = -1;

        for (uint32_t i = 0; i < m_TextureSlots.size(); i++)
        {
            if (m_TextureSlots[i] == texture)
            {
                textureIndex = i;
                break;
            }
        }

        if (textureIndex == -1)
        {
            textureIndex = m_TextureSlotIndex;
            m_TextureSlots[m_TextureSlotIndex] = texture;
            m_TextureSlotIndex++;
        }

        m_Uniforms[name] = textureIndex;
    }

    void OpenGLMaterial::Bind()
    {
        BindTextures();
        m_Shader->Bind();
        for (auto uniform : m_Uniforms)
        {
            m_Shader->SetUniform(uniform.first, uniform.second);
        }
    }

    void OpenGLMaterial::BindTextures()
    {
        for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
            m_TextureSlots[i]->Bind(i);
    }

    void OpenGLMaterial::PopulateUniforms(const Ref<OpenGLShader> &shader)
    {
        UniformTable uniformTable = shader->GetUniformTable();
        for (const auto& pair : uniformTable)
            Set(pair.first, GetUniformDefaultValue(pair.second));
    }
}