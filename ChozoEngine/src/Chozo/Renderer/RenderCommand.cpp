#include "RenderCommand.h"

#include "Backend/OpenGL/OpenGLRendererAPI.h"

namespace Chozo {

    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}