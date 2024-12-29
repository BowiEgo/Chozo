#include "RenderPass.h"

#include "RenderCommand.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLRenderPass.h"

namespace Chozo {

    Ref<RenderPass> RenderPass::Create(RenderPassSpecification &spec)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLRenderPass>::Create(spec);
        }
        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}