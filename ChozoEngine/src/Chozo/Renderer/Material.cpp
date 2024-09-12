#include "Material.h"

#include "Renderer.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLMaterial.h"

namespace Chozo {

    Ref<Material> Material::Create(const std::string &name)
    {
        Ref<Shader> shader;
        if (name == "Basic")
            shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Basic");
        else if (name == "Phong")
            shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Phong");
        else
            CZ_CORE_ASSERT(false, "Unknown Material!");

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLMaterial>(shader, name);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Material> Material::Create(Ref<Shader> shader, const std::string &name)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLMaterial>(shader, name);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Material> Material::Copy(const Ref<Material> &other, const std::string &name)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLMaterial>(other, name);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}