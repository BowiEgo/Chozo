#pragma once

#include "GraphicsContext.h"

class RENDER_CORE_API COpenGLContext : public CGraphicsContext {
public:
    COpenGLContext(IRendererWindow *windowHandle);
    ~COpenGLContext() {};

    virtual void Init() override;
    virtual void SwapBuffers() override;

    virtual void CreateRenderer() override {}; // TODO: Remove

private:
    FWindowHandle m_FWindowHandle{};
};