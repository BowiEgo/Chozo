#include "IndexBuffer.h"

#include "RenderCommand.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLIndexBuffer.h"

namespace Chozo {

    Ref<IndexBuffer> IndexBuffer::Create(void *indices, uint32_t count)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLIndexBuffer>::Create(indices, count);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}