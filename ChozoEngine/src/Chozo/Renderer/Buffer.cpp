#include "Buffer.h"

#include "Renderer2D.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Chozo {
    Ref<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size)
    {
        switch (Renderer2D::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t size)
    {
        switch (Renderer2D::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLIndexBuffer>(indices, size);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}