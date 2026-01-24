#include "FEngineLoop.h"
#include "Window/GenericPlatform/GLFWWindow.h"

namespace Chozo {

    DEFINE_LOG_CATEGORY(LogEngineLoop);

    FEngineLoop::FEngineLoop()
    { }

    void FEngineLoop::Init()
	{
        CZ_LOG(LogEngineLoop, Info, "Engine Loop Initializing...");

        FWindowDefinition def;
        def.Title = "Chozo Engine - Vulkan";
        def.Width = 1280;
        def.Height = 720;

        m_Window = std::make_unique<GLFWWindow>(def);
	}

    void FEngineLoop::Tick()
    {
        if (m_Window) {
            m_Window->OnUpdate();
        }
    }

    void FEngineLoop::Exit()
    {
        if (m_Window) {
            m_Window->Shutdown();
        }
    }

    bool FEngineLoop::ShouldClose() const
    {
        return m_Window ? m_Window->ShouldClose() : true;
    }
}
