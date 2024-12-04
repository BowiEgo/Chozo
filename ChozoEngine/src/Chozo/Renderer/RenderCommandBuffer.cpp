#include "RenderCommandBuffer.h"

#include "RenderCommand.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLRenderCommandBuffer.h"

namespace Chozo {
    Ref<RenderCommandBuffer> RenderCommandBuffer::Create()
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLRenderCommandBuffer>::Create();
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}