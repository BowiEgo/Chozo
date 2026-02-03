#pragma once

#include "GraphicsContext.h"

namespace Chozo {

class RENDERCORE_API OpenGLContext : public GraphicsContext {
public:
    OpenGLContext(IRendererWindow *windowHandle);
    ~OpenGLContext() {};

    virtual void Init() override;
    virtual void SwapBuffers() override;

    virtual void CreateRenderer() override {}; // TODO: Remove

private:
    WindowHandle m_WindowHandle{};
};
} // namespace Chozo