#include "RenderCommand.h"

#include "Backend/OpenGL/OpenGLRenderAPI.h"

namespace Chozo {

    RenderAPI::Type RenderCommand::s_Type = RenderAPI::Type::None;
    RenderAPI* RenderCommand::s_API = nullptr;

    void RenderCommand::SwitchAPI(RenderAPI::Type type)
    {
        s_Type = type;

        switch (s_Type)
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!");
            case RenderAPI::Type::OpenGL:   s_API = new OpenGLRenderAPI;
        }
    }
}