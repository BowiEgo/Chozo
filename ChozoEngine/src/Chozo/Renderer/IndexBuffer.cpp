#include "IndexBuffer.h"

#include "Renderer2D.h"
#include "Platform/OpenGL/OpenGLIndexBuffer.h"

namespace Chozo {

    Ref<IndexBuffer> IndexBuffer::Create(void *indices, uint32_t count)
    {
        switch (Renderer2D::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLIndexBuffer>(indices, count);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}