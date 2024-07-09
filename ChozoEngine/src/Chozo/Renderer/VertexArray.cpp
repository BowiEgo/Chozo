#include "VertexArray.h"

#include "Renderer2D.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Chozo {
    VertexArray* VertexArray::Create()
    {
        switch (Renderer2D::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return new OpenGLVertexArray();
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}