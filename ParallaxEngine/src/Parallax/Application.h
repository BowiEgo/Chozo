#pragma once

#include "prxpch.h"
#include "Core.h"
#include "Events/Event.h"
#include "Parallax/Events/ApplicationEvent.h"

#include "Window.h"

namespace Parallax
{
    class PARALLAX_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);
    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}