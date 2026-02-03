#include "FEngineLoop.h"
#include "RendererAPI.h"

namespace Chozo {

DEFINE_LOG_CATEGORY(LogEngineLoop);

FEngineLoop::FEngineLoop() {}

void FEngineLoop::Init() {
    CZ_LOG(LogEngineLoop, Trace, "Engine Loop Initializing...");

    fs::path projectRoot = Utils::File::GetProjectRoot();
    CZ_LOG(LogEngineLoop, Info,
           "Project Root set from environment variable: {0}",
           projectRoot.string());
    VFS::SetProtocolPath("engine", projectRoot);
    VFS::SetProtocolPath("shaders", projectRoot / "Shaders");

    RendererAPI::SetAPI(RendererAPI::API::Vulkan);

    FWindowDefinition def;
    def.Title = "Chozo Engine - Vulkan";
    def.Width = 1280;
    def.Height = 720;

    m_Window = Window::Create(def);
    CZ_ASSERT(m_Window, "Failed to create window!");
    m_Window->Init();

    m_RenderEngine = CreateScope<RenderEngine>(m_Window.get());
    m_RenderEngine->Init();

    CZ_LOG(LogEngineLoop, Info, "Engine Loop Initialized");
}

void FEngineLoop::Tick() {
    if (m_Window) {
        m_Window->OnUpdate();
    }
}

void FEngineLoop::Exit() {
    if (m_Window) {
        m_Window->Shutdown();
    }
}

bool FEngineLoop::ShouldClose() const {
    return m_Window ? m_Window->ShouldClose() : true;
}
} // namespace Chozo
