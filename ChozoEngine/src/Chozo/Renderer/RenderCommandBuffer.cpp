#include "RenderCommandBuffer.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLRenderCommandBuffer.h"

namespace Chozo {
    Ref<RenderCommandBuffer> RenderCommandBuffer::Create()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLRenderCommandBuffer>();
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}