#include "OpenGLContext.h"

namespace Chozo {

OpenGLContext::OpenGLContext(IRendererWindow *windowHandle)
    : GraphicsContext(windowHandle) {}

void OpenGLContext::Init() {}

void OpenGLContext::SwapBuffers() {
    // Present the rendered image to the screen}
}
} // namespace Chozo