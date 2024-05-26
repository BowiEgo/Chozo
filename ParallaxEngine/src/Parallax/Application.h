#pragma once

#include "Core.h"

#include "Window.h"
#include "Parallax/LayerStack.h"
#include "Parallax/Events/Event.h"
#include "Parallax/Events/ApplicationEvent.h"

#include "Parallax/ImGui/ImGuiLayer.h"

namespace Parallax
{
    class PARALLAX_API Application
    {
    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        LayerStack m_LayerStack;

        unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
    private:
        static Application* s_Instance;
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline Window& GetWindow() { return *m_Window; }
        inline static Application& Get() { return *s_Instance; }
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}