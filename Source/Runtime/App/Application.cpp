#include <Runtime/App/Application.h>

#include <Core/Header/Assert.h>
#include <Core/Header/Macros.h>

#include <iostream>

namespace CZ {

DEFINE_LOG_CATEGORY(LogApplication);

Application::Application() {}

Application::~Application() {}

bool Application::Startup(const ApplicationSpecification& appSpec, std::string& err) {
    // Setup Window
    {
        WindowSpecifaciton spec;
        spec.Title = appSpec.Name;
        spec.Size  = { WINDOW_WIDTH, WINDOW_HEIGHT };

        m_Window = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, Window, spec);
        m_Window->Init(err);
        m_Window->SetEventCallback(CZ_BIND_EVENT_FN(OnEvent));
    }

    {
        // Setup Engine
        m_Engine = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, Engine, m_Window.get());
        m_Engine->Init(err);
    }

    return true;
}

void Application::Shutdown() {
    m_Engine->Shutdown();
    m_Engine.reset();

    m_Window->Shutdown();
    m_Window.reset();

    ReportMemoryLeaks();
}

void Application::Run() {
    // CZ_LOG(LogApplication, Trace, "Running...");
    m_Window->OnUpdate();
}

bool Application::OnEvent(Event& e) {
    // for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
    //     (*--it)->OnEvent(e);
    //     if (e.isHandled()) break;
    // }
    // return m_RenderEngine->OnEvent(e);
    return true;
}

} // namespace CZ