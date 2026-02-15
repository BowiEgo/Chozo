#include "EngineLoop.h"
#include "RendererAPI.h"

DEFINE_LOG_CATEGORY(LogEngineLoop);

CEngineLoop::CEngineLoop() {}

void CEngineLoop::Init() {
    CZ_LOG(LogEngineLoop, Trace, "Engine Loop Initializing...");

    std::filesystem::path projectRoot = ChozoUtils::File::GetProjectRoot();
    CZ_LOG(LogEngineLoop, Info, "Project Root set from environment variable: {0}",
           projectRoot.string());
    VFS::SetProtocolPath("engine", projectRoot);
    VFS::SetProtocolPath("shaders", projectRoot / "Shaders");

    CRendererAPI::SetType(CRendererAPI::EType::Vulkan);

    FWindowDefinition def;
    def.Title = "Chozo Engine - Vulkan";
    def.Width = WINDOW_WIDTH;
    def.Height = WINDOW_HEIGHT;

    m_Window = CWindow::Create(def);
    CZ_CORE_ASSERT(m_Window, "App: Failed to create window!");
    m_Window->Init();
    m_Window->SetEventCallback(CZ_BIND_EVENT_FN(OnEvent));

    m_RenderEngine = CreateScope<CRenderEngine>(m_Window.get());
    m_RenderEngine->Init();

    CZ_LOG(LogEngineLoop, Info, "Engine Loop Initialized");
}

void CEngineLoop::Tick() {
    m_Window->OnUpdate();
    m_RenderEngine->Tick();
}

void CEngineLoop::Exit() {
    m_RenderEngine->Shutdown();
    m_Window->Shutdown();
}

bool CEngineLoop::ShouldClose() const { return m_Window ? m_Window->ShouldClose() : true; }

bool CEngineLoop::OnEvent(IEvent& e) { return m_RenderEngine->OnEvent(e); }
