#include "VertexBuffer.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

namespace Chozo {

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLVertexBuffer>(size);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(void *vertices, uint32_t size)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}