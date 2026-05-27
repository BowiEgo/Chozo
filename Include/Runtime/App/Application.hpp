#pragma once

#include <Core/Layer/LayerStack.hpp>
#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/App/Engine.hpp>
#include <Runtime/App/StartupHost.hpp>
#include <Runtime/Window/Window.hpp>

#include <string>

namespace CZ {

DECLARE_LOG_CATEGORY_EXTERN(LogApplication, Info);

#ifndef WINDOW_WIDTH
    #define WINDOW_WIDTH 2560
#endif

#ifndef WINDOW_HEIGHT
    #define WINDOW_HEIGHT 1920
#endif

struct ApplicationSpecification {
    std::string Name = "Chozo Engine";
};

class Application {
public:
    static Application& Get();

    explicit Application();
    Application(const Application&) = delete;
    ~Application();

    Application& operator=(const Application&) = delete;

    bool Startup(const ApplicationSpecification& spec, std::string& err);
    bool ShouldClose() { return m_Window->ShouldClose() || m_ShouldClose; }
    void Close() { m_ShouldClose = true; }
    void Shutdown();
    void Run();
    bool OnEvent(Event& e);

    StartupHost GetStartupHost() { return m_StartupHost; }
    void SetStartupHost(StartupHost StartupHost) { m_StartupHost = StartupHost; }

    Window GetWindow() { return m_Window; }
    Window GetWindow() const { return m_Window; }

    Engine* GetEngine() { return m_Engine.get(); }

private:
    bool m_ShouldClose = false;
    Window m_Window;
    Scope<Engine> m_Engine;

    StartupHost m_StartupHost;
    LayerStack m_LayerStack;
};

} // namespace CZ
