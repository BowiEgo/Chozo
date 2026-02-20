#include "Application.h"
#include "RendererAPI.h"

DEFINE_LOG_CATEGORY(LogApplication);

CApplication* CApplication::s_Instance = nullptr;

CApplication::CApplication(const std::string& name) {
    CZ_CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;
    Init(name);
}

void CApplication::Init(const std::string& name) {
    CZ_LOG(LogApplication, Trace, "Engine Loop Initializing...");

    std::filesystem::path projectRoot = ChozoUtils::File::GetProjectRoot();
    CZ_LOG(LogApplication, Info, "Project Root set from environment variable: {0}",
           projectRoot.string());
    std::filesystem::path resourcesDir = projectRoot / "Resources";
    VFS::Mount("engine", projectRoot);
    VFS::Mount("shaders", projectRoot / "Shaders");
    VFS::Mount("resources", resourcesDir);
    VFS::Mount("fonts", resourcesDir / "Fonts");

    FRendererAPI::SetType(FRendererAPI::EType::Vulkan);

    FWindowDefinition def;
    def.Title = name;
    def.Width = WINDOW_WIDTH;
    def.Height = WINDOW_HEIGHT;

    m_Window = CWindow::Create(def);
    CZ_CORE_ASSERT(m_Window, "App: Failed to create window!");
    m_Window->Init();
    m_Window->SetEventCallback(CZ_BIND_EVENT_FN(OnEvent));

    m_RenderEngine = CreateScope<CRenderEngine>(m_Window.get());
    m_RenderEngine->Init();

    m_ImGuiLayer = new CImGuiLayer(m_Window.get(), m_RenderEngine->GetRenderer());
    PushLayer(m_ImGuiLayer);

    m_RenderEngine->GetRenderer()->SetUICallback([this](const TRef<IRHICommandBuffer>& cmdBuffer) {
        if (m_ImGuiLayer)
            m_ImGuiLayer->Render(cmdBuffer);
    });

    CZ_LOG(LogApplication, Info, "Engine Loop Initialized");
}

void CApplication::Run() {
    m_Window->OnUpdate();

    m_ImGuiLayer->Begin();
    for (ILayer* layer : m_LayerStack)
        layer->OnImGuiRender();
    m_ImGuiLayer->End();

    m_RenderEngine->Tick();
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
