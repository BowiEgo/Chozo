#pragma once

#include "Core.h"
#include "GraphicsContext.h"
#include "Window.h"

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogRenderEngine, Info);

class ENGINE_API RenderEngine {
public:
    RenderEngine(Window* window) : m_Window(window) {};
    ~RenderEngine() = default;

    GraphicsContext &GetGraphicsContext() { return *m_Context; }

    void Init();

private:
    Scope<GraphicsContext> m_Context;
    Window* m_Window;
};

} // namespace Chozo