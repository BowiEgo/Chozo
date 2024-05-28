#include "VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Parallax {
    VertexArray* VertexArray::Create()
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::None:     PRX_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::OpenGL:   return new OpenGLVertexArray();
        }

        PRX_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}