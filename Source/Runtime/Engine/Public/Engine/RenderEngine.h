#pragma once

#include "CoreMinimal.h"
#include "EngineExport.h"
#include "GraphicsContext.h"
#include "Scope.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderEngine, Info);

class ENGINE_API CRenderEngine {
public:
    CRenderEngine(CWindow* window) : m_Window(window) {};
    ~CRenderEngine();

    void Init();

private:
    CWindow* m_Window;
    TScope<CGraphicsContext> m_Context;
};
