#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Chozo {

    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}