#pragma once

#include "Core.h"
#include "EngineExport.h"
#include "Event.h"
#include "FPSCounter.h"
#include "ImGuiLayer.h"
#include "LayerStack.h"
#include "Module.h"
#include "RenderEngine.h"
#include "Scope.h"
#include "Timer.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogApplication, Info);

#if 1
    // Create a unique timer variable named e.g., timer123
    #define CZ_APP_SCOPE_PERF(name)                                                                \
        ScopePerfTimer CZ_CONCAT(timer, __LINE__)(name,                                            \
                                                  CApplication::Get()->GetPerformanceProfiler());

    #define CZ_APP_SCOPE_TIMER(name) ScopedTimer CZ_CONCAT(timer, __LINE__)(name);
#else
    #define CZ_APP_SCOPE_PERF(name)
    #define CZ_APP_SCOPE_TIMER(name)
#endif

enum class EAppPowerMode {
    NoLimit,     // Run as fast as possible with no frame cap
    Extreme,     // Cap at very high framerate (e.g., 500 fps)
    Performance, // Cap at moderate framerate (e.g., 200 fps) for good performance without
                 // overheating
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

    void SetPowerMode(EAppPowerMode mode) {
        m_PowerMode = mode;
        if (m_IsMinimized) {
            m_TargetFrameTime = 1000.0f / 15.0f;
        } else {
            switch (m_PowerMode) {
                case EAppPowerMode::NoLimit: m_TargetFrameTime = 0.0f; break;
                case EAppPowerMode::Extreme:
                    m_TargetFrameTime = 1000.0f / 500.0f;
                    break; // 500 FPS cap to prevent extreme CPU usage
                case EAppPowerMode::Performance:
                    m_TargetFrameTime = 1000.0f / 200.0f;
                    break;                                                                // 200 FPS
                case EAppPowerMode::Balanced: m_TargetFrameTime = 1000.0f / 60.0f; break; // 60 FPS
                case EAppPowerMode::PowerSaving:
                    m_TargetFrameTime = 1000.0f / 30.0f;
                    break; // 30 FPS
            }
        }
    }

    CWindow* GetWindow() const { return m_Window.get(); }
    CRenderEngine* GetRenderEngine() const { return m_RenderEngine.get(); }
    PerformanceProfiler* GetPerformanceProfiler() { return m_Profiler.get(); }
    GPUProfiler GetGPUProfiler() {
        return m_RenderEngine->GetRenderer()->GetGraphicContext()->GetDevice()->GetProfiler();
    }
    FPSCounter* GetFPSCounter() { return &m_FPSCounter; }
    EAppPowerMode GetPowerMode() { return m_PowerMode; }
    CImGuiLayer& GetImGuiLayer() const { return *m_ImGuiLayer; }
    uint32 GetCurrentFrameIndex() const {
        return m_RenderEngine->GetRenderer()->GetGraphicContext()->GetCurrentFrameIndex();
    }

    static CApplication* Get() { return s_Instance; }

private:
    bool OnEvent(IEvent& e);

private:
    static CApplication* s_Instance;

    CModule m_EditorModule;
    CModule m_SandboxModule;

    Timer m_AppTimer;
    float m_LastFrameTime     = 0.0f;
    EAppPowerMode m_PowerMode = EAppPowerMode::Balanced;
    float m_TargetFrameTime   = 0.0f;
    bool m_IsMinimized        = false;
    TScope<PerformanceProfiler> m_Profiler;
    FPSCounter m_FPSCounter;

    TScope<CWindow> m_Window;
    TScope<CRenderEngine> m_RenderEngine;

    CImGuiLayer* m_ImGuiLayer;
    ILayerStack m_LayerStack;
};
