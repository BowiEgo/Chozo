#include "Core/Memory/Memory.h"
#include "Runtime/Window/Window.h"
#include <Runtime/App/Application.h>

#include <Core/Header/Assert.h>
#include <Core/Header/Macros.h>

#include <iostream>

namespace CZ {

DEFINE_LOG_CATEGORY(LogApplication);

Application& Application::Get() {
    static Application Instance;
    return Instance;
}

Application::Application() {}

Application::~Application() {}

bool Application::Startup(const ApplicationSpecification& appSpec, std::string& err) {
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

    m_LayerStack.PushLayer(m_StartupLayer);

    // // Setup UI
    // {
    //     m_UI = CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, UI, m_Engine.get());
    //     m_UI->Init(err);
    // }

    return true;
}

void Application::Shutdown() {
    m_LayerStack.Clear();

    m_Engine->Shutdown();
    m_Engine.reset();

    m_Window.Shutdown();
    Window::Destroy(m_Window);

    ReportMemoryLeaks();
}

void Application::Run() {
    // CZ_LOG(LogApplication, Trace, "Running...");
    m_Window.OnUpdate();

    float deltaTime = 0.1;

    for (Layer* layer : m_LayerStack)
        layer->OnUpdate(deltaTime);

    for (Layer* layer : m_LayerStack)
        layer->OnRender();

    // m_UI->OnUpdate();
    // m_UI->Render();
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