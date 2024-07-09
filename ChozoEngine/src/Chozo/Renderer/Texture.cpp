#include "Texture.h"

#include "Renderer2D.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"

namespace Chozo
{
    Ref<Texture2D> Texture2D::Create(const std::string &path)
    {
        switch (Renderer2D::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLTexture2D>(path);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
