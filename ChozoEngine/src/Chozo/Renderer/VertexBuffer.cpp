#include "VertexBuffer.h"

#include "RenderCommand.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLVertexBuffer.h"

namespace Chozo {

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLVertexBuffer>::Create(size);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(void *vertices, uint32_t size)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLVertexBuffer>::Create(vertices, size);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}