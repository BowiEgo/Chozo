#include "Framebuffer.h"

#include "RendererAPI.h"
#include "Backend/OpenGL/OpenGLFramebuffer.h"

namespace Chozo {
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLFramebuffer>::Create(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}