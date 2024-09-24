#include "Shader.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Chozo {

    Ref<Shader> Shader::Create(const ShaderSpecification &spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLShader>::Create(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLShader>::Create(name, vertexSrc, fragmentSrc);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    void ShaderLibrary::Load(std::string_view name, const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        Ref<Shader> shader = Shader::Create(std::string(name), vertexSrc, fragmentSrc);
        m_Shaders.emplace(name, shader);
    }

    const Ref<Shader>& ShaderLibrary::Get(const std::string &name) const
    {
		CZ_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "");
        return m_Shaders.at(name);
    }

    Ref<ShaderLibrary> ShaderLibrary::Create()
    {
        return Ref<ShaderLibrary>::Create();
    }
}