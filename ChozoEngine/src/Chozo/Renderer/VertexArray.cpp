#include "VertexArray.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Chozo {
    Ref<VertexArray> VertexArray::Create()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLVertexArray>();
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}