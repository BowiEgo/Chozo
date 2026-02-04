#include "EngineLoop.h"
#include "RendererAPI.h"

DEFINE_LOG_CATEGORY(LogEngineLoop);

CEngineLoop::CEngineLoop() {}

void CEngineLoop::Init() {
    CZ_LOG(LogEngineLoop, Trace, "Engine Loop Initializing...");

    std::filesystem::path projectRoot = ChozoUtils::File::GetProjectRoot();
    CZ_LOG(LogEngineLoop, Info,
           "Project Root set from environment variable: {0}",
           projectRoot.string());
    VFS::SetProtocolPath("engine", projectRoot);
    VFS::SetProtocolPath("shaders", projectRoot / "Shaders");

    CRendererAPI::SetType(CRendererAPI::EType::Vulkan);

    FWindowDefinition def;
    def.Title = "Chozo Engine - Vulkan";
    def.Width = 1280;
    def.Height = 720;

    m_Window = CWindow::Create(def);
    CZ_CORE_ASSERT(m_Window, "App: Failed to create window!");
    m_Window->Init();

    m_RenderEngine = CreateScope<CRenderEngine>(m_Window.get());
    m_RenderEngine->Init();

    CZ_LOG(LogEngineLoop, Info, "Engine Loop Initialized");
}

void CEngineLoop::Tick() {
    if (m_Window) {
        m_Window->OnUpdate();
    }
}

void CEngineLoop::Exit() {
    if (m_Window) {
        m_Window->Shutdown();
    }
}

bool CEngineLoop::ShouldClose() const {
    return m_Window ? m_Window->ShouldClose() : true;
}
