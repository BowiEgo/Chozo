#include "Application.h"

#include "RendererAPI.h"

#ifdef CZ_PLATFORM_WINDOWS
    #include <Windows.h>
    #include <timeapi.h>
    // Link the WinMM library for timer functions
    #pragma comment(lib, "Winmm.lib")
#endif

DEFINE_LOG_CATEGORY(LogApplication);

CApplication* CApplication::s_Instance = nullptr;

CApplication::CApplication(const std::string& name) {
    CZ_CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;
    Init(name);
}

CApplication::~CApplication() {
#ifdef CZ_PLATFORM_WINDOWS
    timeEndPeriod(1);
#endif
}

void CApplication::Init(const std::string& name) {
    CZ_LOG(LogApplication, Trace, "Applicatin Initializing...");

    m_Profiler = CreateScope<PerformanceProfiler>();
    {
        CZ_SCOPE_PERF("Application_Init_Time");

#ifdef CZ_PLATFORM_WINDOWS
        // Set the timer resolution to 1ms for high-precision sleep
        timeBeginPeriod(1);
#endif

        // Setup VFS
        std::filesystem::path projectRoot = ChozoUtils::File::GetProjectRoot();
        CZ_LOG(LogApplication, Info, "Project Root set from environment variable: {0}",
               projectRoot.string());
        std::filesystem::path resourcesDir = projectRoot / "Resources";
        VFS::Mount("engine", projectRoot);
        VFS::Mount("shaders", projectRoot / "Shaders");
        VFS::Mount("resources", resourcesDir);
        VFS::Mount("fonts", resourcesDir / "Fonts");

        FRendererAPI::SetType(FRendererAPI::EType::Vulkan);

        // Setup Window
        FWindowDefinition def;
        def.Title = name;
        def.Width = WINDOW_WIDTH;
        def.Height = WINDOW_HEIGHT;

        m_Window = CWindow::Create(def);
        CZ_CORE_ASSERT(m_Window, "App: Failed to create window!");
        m_Window->Init();
        m_Window->SetEventCallback(CZ_BIND_EVENT_FN(OnEvent));

        // Setup RenderEngine
        m_RenderEngine = CreateScope<CRenderEngine>(m_Window.get());
        m_RenderEngine->Init();

        m_ImGuiLayer = new CImGuiLayer(m_Window.get(), m_RenderEngine->GetRenderer());
        PushLayer(m_ImGuiLayer);

        m_RenderEngine->GetRenderer()->SetUICallback(
            [this](const TRef<IRHICommandBuffer>& cmdBuffer) {
                if (m_ImGuiLayer) m_ImGuiLayer->Render(cmdBuffer);
            });
    }

    CZ_LOG(LogApplication, Info, "Applicatin Initialized");
}

void CApplication::Run() {
    CZ_SCOPE_PERF("Total FrameTime");

    float targetFrameTime = 0.0f;
    if (m_IsMinimized) {
        targetFrameTime = 1000.0f / 15.0f;
    } else {
        switch (m_PowerMode) {
        case EAppPowerMode::Performance:
            targetFrameTime = 0.0f;
            break;
        case EAppPowerMode::Balanced:
            targetFrameTime = 16.67f;
            break; // 60 FPS
        case EAppPowerMode::PowerSaving:
            targetFrameTime = 33.33f;
            break; // 30 FPS
        }
    }

    Timer frameTimer;
    frameTimer.Reset();

    // Tick
    m_Window->OnUpdate();
    m_ImGuiLayer->Begin();
    for (ILayer* layer : m_LayerStack)
        layer->OnImGuiRender();
    m_ImGuiLayer->End();
    m_RenderEngine->Tick();

    float elapsed = frameTimer.ElapsedMillis();
    float timeToWait = targetFrameTime - elapsed;
    if (timeToWait > 0) {
        // Log wait time separately to see CPU headroom in Profiler
        CZ_SCOPE_PERF("Wait Time");
        Timer::Wait(timeToWait);
    }
}

void CApplication::Exit() {
    m_LayerStack.Clear();
    m_RenderEngine->Shutdown();
    m_Window->Shutdown();
}

bool CApplication::ShouldClose() const { return m_Window ? m_Window->ShouldClose() : true; }

void CApplication::PushLayer(ILayer* layer) {
    m_LayerStack.PushLayer(layer);
    layer->OnAttach();
}

bool CApplication::OnEvent(IEvent& e) { return m_RenderEngine->OnEvent(e); }
