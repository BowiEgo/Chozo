#include "IndexBuffer.h"

#include "RendererAPI.h"
#include "Backend/OpenGL/OpenGLIndexBuffer.h"

namespace Chozo {

    Ref<IndexBuffer> IndexBuffer::Create(void *indices, uint32_t count)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return Ref<OpenGLIndexBuffer>::Create(indices, count);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}