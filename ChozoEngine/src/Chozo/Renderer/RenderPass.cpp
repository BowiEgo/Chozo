#include "RenderPass.h"

#include "RendererAPI.h"
#include "Backend/OpenGL/OpenGLRenderPass.h"

namespace Chozo {

    Ref<RenderPass> RenderPass::Create(RenderPassSpecification &spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLRenderPass>::Create(spec);
        }
        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}