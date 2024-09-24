#include "Texture.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Chozo
{

    //////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////////////

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLTexture2D>::Create(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const std::string &path, const TextureSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLTexture2D>::Create(path, spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const RendererID &id, const TextureSpecification &spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLTexture2D>::Create(id, spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	//////////////////////////////////////////////////////////////////////////////////

    Ref<TextureCube> TextureCube::Create(const TextureCubeSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLTextureCube>::Create(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
