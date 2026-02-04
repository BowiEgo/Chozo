#pragma once

#include "Core.h"
#include "GraphicsContext.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderEngine, Info);

class ENGINE_API CRenderEngine {
public:
    CRenderEngine(CWindow *window) : m_Window(window) {};
    ~CRenderEngine() = default;

    CGraphicsContext &GetCGraphicsContext() { return *m_Context; }

    void Init();

private:
    TScope<CGraphicsContext> m_Context;
    CWindow *m_Window;
};
