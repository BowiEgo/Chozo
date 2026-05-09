#pragma once

#include <Core/Log/LogMacros.h>
#include <Runtime/App/Engine.h>
#include <Runtime/Window/Window.h>

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
    explicit Application();
    Application(const Application&) = delete;
    ~Application();

    Application& operator=(const Application&) = delete;

    bool Startup(const ApplicationSpecification& spec, std::string& err);
    bool ShouldClose() const { return m_ShouldClose; }

    void Shutdown();

    void Run();

    bool OnEvent(Event& e);

private:
    bool m_ShouldClose = false;
    Scope<Window> m_Window;
    Scope<Engine> m_Engine;
};

} // namespace CZ
