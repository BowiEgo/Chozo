#include "Runtime/Window/Window.hpp"
#include <Runtime/App/Application.hpp>

#include <Core/FileSystem/VFS.hpp>
#include <Core/Header/Assert.hpp>
#include <Core/Header/Macros.h>
#include <Core/Platform/Platform.h>

namespace CZ {

DEFINE_LOG_CATEGORY(LogApplication);

Application& Application::Get() {
    static Application Instance;
    return Instance;
}

Application::Application() {}

Application::~Application() {}

bool Application::Startup(const ApplicationSpecification& appSpec, std::string& err) {
    // Setup VFS
    std::filesystem::path projectRoot =
        std::filesystem::absolute(Platform::File::GetExecutablePath()).parent_path();
    CZ_APP_LOG(Info, "Project Root set from environment variable: {0}", projectRoot.string());
    std::filesystem::path resourcesDir = projectRoot / "../Resources";
    VFS::Mount("engine", projectRoot);
    VFS::Mount("resources", resourcesDir);
    VFS::Mount("shaders", resourcesDir / "Shaders");
    VFS::Mount("fonts", resourcesDir / "Fonts");
    VFS::Mount("svgs", resourcesDir / "SVGs");
    VFS::Mount("textures", resourcesDir / "Textures");

    // Setup Window
    {
        WindowSpecifaciton spec;
        spec.Title = appSpec.Name;
        spec.Size  = { WINDOW_WIDTH, WINDOW_HEIGHT };

        m_Window = Window::Create(spec);
        m_Window.Init(err);
        m_Window.SetEventCallback(CZ_BIND_FN(OnEvent));
    }

    // Setup Engine
    {
        m_Engine = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, Engine);
        m_Engine->Init(err);
    }

    m_LayerStack.PushLayer(m_StartupHost.GetStartupLayer());
    if (m_StartupHost.IsOffscreen()) {
        m_Engine->GetRenderer().SetDrawFuncToFinalPass(CZ_BIND_FN(m_StartupHost.Draw));
    }

    return true;
}

void Application::Shutdown() {
    m_StartupHost.Destroy();
    m_LayerStack.Clear();

    m_Engine->Shutdown();
    m_Engine.reset();

    m_Window.Shutdown();
    m_Window.Destroy();

    ReportMemoryLeaks();
}

void Application::Run() {
    // CZ_APP_LOG( Trace, "Running...");
    m_Window.OnUpdate();

    float deltaTime = 0.1;

    for (Layer* layer : m_LayerStack)
        layer->OnUpdate(deltaTime);

    for (Layer* layer : m_LayerStack)
        layer->OnRender();

    m_Engine->Tick(deltaTime);
}

bool Application::OnEvent(Event& e) {
    for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
        (*--it)->OnEvent(e);
        if (e.isHandled()) break;
    }
    return m_Engine->OnEvent(e);
    return true;
}

} // namespace CZ