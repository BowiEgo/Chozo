#include "OpenGLContext.h"

COpenGLContext::COpenGLContext(IRendererWindow *windowHandle)
    : CGraphicsContext(windowHandle) {}

void COpenGLContext::Init() {}

void COpenGLContext::SwapBuffers() {
    // Present the rendered image to the screen}
}