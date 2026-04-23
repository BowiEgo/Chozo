#include "Application.h"

#include "GLFWInputImpl.h"
#include "Input.h"
#include "ModuleUtils.h"
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
        VFS::Mount("svgs", resourcesDir / "SVGs");
        VFS::Mount("textures", resourcesDir / "Textures");

        FRendererAPI::SetType(FRendererAPI::EType::Vulkan);

        // Setup Window
        FWindowDefinition def;
        def.Title = name;
        def.Size  = { WINDOW_WIDTH, WINDOW_HEIGHT };

        m_Window = CWindow::Create(def);
        CZ_CORE_ASSERT(m_Window, "App: Failed to create window!");
        m_Window->Init();
        m_Window->SetEventCallback(CZ_BIND_EVENT_FN(OnEvent));

        auto* inputImpl = new CGLFWInputImpl(m_Window.get());
        SInput::Init(inputImpl);

        // Setup RenderEngine
        m_RenderEngine = CreateScope<CRenderEngine>(m_Window.get());
        m_RenderEngine->Init();

        m_ImGuiLayer =
            new CImGuiLayer(m_Window.get(), m_RenderEngine->GetRenderer()->GetGraphicContext());
        PushLayer(m_ImGuiLayer);

        m_RenderEngine->GetRenderer()->SetUICallback(
            [this](const TRef<IRHICommandList>& cmdBuffer) {
                if (m_ImGuiLayer) m_ImGuiLayer->Draw(cmdBuffer);
            });

        if (m_EditorModule.Load(ChozoUitls::Module::GetPlatformLibName("Editor"))) {
            auto EditorLayer = m_EditorModule.Invoke<ILayer*()>("CreateEditorLayer");
            PushLayer(EditorLayer);
        }

        // if (m_SandboxModule.Load(ChozoUitls::Module::GetPlatformLibName("Sandbox"))) {
        //     auto SandLayer = m_SandboxModule.Invoke<ILayer*()>("CreateSandboxLayer");
        //     PushLayer(SandLayer);
        // }
    }

    SetPowerMode(EAppPowerMode::Balanced);

    CZ_LOG(LogApplication, Info, "Applicatin Initialized");
}

void CApplication::Run() {
    m_Profiler->Flip();
    CZ_APP_SCOPE_PERF(EProfileSlot::TotalFrame);

    float time      = m_AppTimer.ElapsedMillis();
    float deltaTime = time - m_LastFrameTime;
    m_LastFrameTime = time;

    m_FPSCounter.Update(deltaTime);

    {
        CZ_APP_SCOPE_PERF(EProfileSlot::Logic);

        m_Window->OnUpdate();
        for (ILayer* layer : m_LayerStack)
            layer->OnUpdate(deltaTime);

        {
            // TODO: execute this stuff on render thread.
            CZ_APP_SCOPE_PERF(EProfileSlot::ImGui);
            m_ImGuiLayer->Begin();
            m_ImGuiLayer->Render([this]() {
                for (ILayer* layer : m_LayerStack)
                    layer->OnImGuiRender();
            });
            m_ImGuiLayer->End();
        }

        {
            CZ_APP_SCOPE_PERF(EProfileSlot::Render);
            m_RenderEngine->Tick(deltaTime);
        }
    }

    float workElapsed = m_AppTimer.ElapsedMillis() - time;
    float timeToWait  = m_TargetFrameTime - workElapsed;
    if (timeToWait > 0) {
        // Log wait time separately to see CPU headroom in Profiler
        CZ_APP_SCOPE_PERF(EProfileSlot::Wait);
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

bool CApplication::OnEvent(IEvent& e) {
    for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
        (*--it)->OnEvent(e);
        if (e.isHandled()) break;
    }
    return m_RenderEngine->OnEvent(e);
}
