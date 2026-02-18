#pragma once

#include "Core.h"
#include "EngineExport.h"
#include "Event.h"
#include "ImGuiLayer.h"
#include "LayerStack.h"
#include "RenderEngine.h"
#include "Scope.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogApplication, Info);

class ENGINE_API CApplication {
public:
    CApplication();
    virtual ~CApplication() {};

    void Init();
    void Run();
    void Exit();

    bool ShouldClose() const;
    void PushLayer(ILayer* layer);

    CWindow* GetWindow() const { return m_Window.get(); }
    CRenderEngine* GetRenderEngine() const { return m_RenderEngine.get(); }

    static CApplication& Get() { return *s_Instance; }

private:
    bool OnEvent(IEvent& e);

private:
    static CApplication* s_Instance;

    TScope<CWindow> m_Window;
    TScope<CRenderEngine> m_RenderEngine;

    CImGuiLayer* m_ImGuiLayer;
    ILayerStack m_LayerStack;
};
