#pragma once

#include "Core.h"

#include "Window.h"
#include "Parallax/LayerStack.h"
#include "Parallax/Events/Event.h"
#include "Parallax/Events/ApplicationEvent.h"

#include "Parallax/ImGui/ImGuiLayer.h"

#include "Parallax/Renderer/Shader.h"
#include "Parallax/Renderer/VertexArray.h"
#include "Parallax/Renderer/Buffer.h"

#include "Parallax/Renderer/OrthographicCamera.h"

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

        inline Window& GetWindow() { return *m_Window; }
        inline static Application& Get() { return *s_Instance; }
    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        LayerStack m_LayerStack;
    private:
        static Application* s_Instance;
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}