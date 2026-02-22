#pragma once

#include "Core.h"
#include "EngineExport.h"
#include "Event.h"
#include "ImGuiLayer.h"
#include "LayerStack.h"
#include "RenderEngine.h"
#include "Scope.h"
#include "Timer.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogApplication, Info);

#define CZ_CONCAT_IMPL(a, b) a##b
#define CZ_CONCAT(a, b) CZ_CONCAT_IMPL(a, b)

#if 1
    // Create a unique timer variable named e.g., timer123
    #define CZ_SCOPE_PERF(name)                                                                    \
        ScopePerfTimer CZ_CONCAT(timer, __LINE__)(name,                                            \
                                                  CApplication::Get()->GetPerformanceProfiler());

    #define CZ_SCOPE_TIMER(name) ScopedTimer CZ_CONCAT(timer, __LINE__)(name);
#else
    #define CZ_SCOPE_PERF(name)
    #define CZ_SCOPE_TIMER(name)
#endif

enum class EAppPowerMode {
    Performance, // No waiting, run as fast as possible
    Balanced,    // Cap at monitor refresh rate (e.g., 60/144 fps)
    PowerSaving  // Cap at low framerate (e.g., 30 fps) or when inactive
};

class ENGINE_API CApplication {
public:
    explicit CApplication(const std::string& name = "Chozo Engine");
    virtual ~CApplication();

    void Init(const std::string& name);
    void Run();
    void Exit();

    bool ShouldClose() const;
    void PushLayer(ILayer* layer);

    void SetPowerMode(EAppPowerMode mode) { m_PowerMode = mode; }

    CWindow* GetWindow() const { return m_Window.get(); }
    CRenderEngine* GetRenderEngine() const { return m_RenderEngine.get(); }
    PerformanceProfiler* GetPerformanceProfiler() { return m_Profiler.get(); }

    static CApplication* Get() { return s_Instance; }

private:
    bool OnEvent(IEvent& e);

private:
    static CApplication* s_Instance;

    EAppPowerMode m_PowerMode = EAppPowerMode::Balanced;
    bool m_IsMinimized = false;
    TScope<PerformanceProfiler> m_Profiler;

    TScope<CWindow> m_Window;
    TScope<CRenderEngine> m_RenderEngine;

    CImGuiLayer* m_ImGuiLayer;
    ILayerStack m_LayerStack;
};

CApplication* CreateApplication();