#include "RenderCommandBuffer.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLRenderCommandBuffer.h"

namespace Chozo {
    Ref<RenderCommandBuffer> RenderCommandBuffer::Create()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLRenderCommandBuffer>::Create();
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}