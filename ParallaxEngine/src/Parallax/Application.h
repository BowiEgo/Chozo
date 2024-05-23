#pragma once

#include "Core.h"

#include "Window.h"
#include "Parallax/LayerStack.h"
#include "Parallax/Events/Event.h"
#include "Parallax/Events/ApplicationEvent.h"

namespace Parallax
{
    class PARALLAX_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
        LayerStack m_LayerStack;
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}