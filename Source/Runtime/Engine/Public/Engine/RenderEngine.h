#pragma once

#include "ApplicationEvent.h"
#include "CoreMinimal.h"
#include "EngineExport.h"
#include "Renderer.h"
#include "Scope.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderEngine, Info);

class ENGINE_API CRenderEngine {
public:
    CRenderEngine(CWindow* window) : m_Window(window) {};
    ~CRenderEngine();

    void Init();
    void Tick();
    void Shutdown();
    bool OnEvent(IEvent& e);
    void OnWindowResize(CWindowResizedEvent& e);

private:
    CWindow* m_Window;
    TScope<CRenderer> m_Renderer;
};
