#include "Material.h"

#include "Renderer.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLMaterial.h"

namespace Chozo {

    //==============================================================================
	///  Material
    Ref<Material> Material::Create(const std::string &name)
    {
        Ref<Shader> shader;
        if (name == "Basic")
            shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Basic");
        if (name == "Solid")
            shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Solid");
        else if (name == "PBR")
            shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Geometry");
        else
            CZ_CORE_ASSERT(false, "Unknown Material!");

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLMaterial>::Create(shader, name);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Material> Material::Create(Ref<Shader> shader, const std::string &name)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLMaterial>::Create(shader, name);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Material> Material::Copy(const Ref<Material> &other, const std::string &name)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLMaterial>::Create(other, name);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    //==============================================================================
    ///  MaterialTable

    MaterialTable::MaterialTable(uint32_t materialCount)
        : m_MaterialCount(materialCount)
    {
    }

    bool MaterialTable::HasMaterial(AssetHandle handle)
    {
        bool exists = false;
        for (const auto& [index, handle] : m_Materials)
        {
            if (handle == handle)
            {
                exists = true;
                break;
            }
        }

        return exists;
    }

    uint32_t MaterialTable::SetMaterial(AssetHandle handle)
    {
        return SetMaterial(m_MaterialCount, handle);
    }

    uint32_t MaterialTable::SetMaterial(uint32_t index, AssetHandle handle)
    {
        m_Materials[index] = handle;
        if (index >= m_MaterialCount)
            m_MaterialCount = index + 1;

        return index;
    }

    inline void MaterialTable::RemoveMaterial(uint32_t index)
    {
        m_Materials.erase(index);
        if (!m_Materials.empty())
            m_MaterialCount = m_Materials.rbegin()->first + 1;
        else
            m_MaterialCount = 0;
    }
}