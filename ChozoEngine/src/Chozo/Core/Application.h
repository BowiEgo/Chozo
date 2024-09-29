#pragma once

#include "Chozo/Core/Base.h"
#include "Chozo/Core/Timestep.h"
#include "Chozo/Core/Window.h"
#include "Chozo/Core/LayerStack.h"

#include "Chozo/Events/Event.h"
#include "Chozo/Events/ApplicationEvent.h"

#include "Chozo/ImGui/ImGuiLayer.h"

namespace Chozo
{
    class CHOZO_API Application
    {
    public:
        Application(const std::string& name);
        virtual ~Application();

        void Run();
        void Close();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline Window& GetWindow() { return *m_Window; }
        inline ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
        inline static Application& Get() { return *s_Instance; }
    private:
        bool OnWindowClose(WindowCloseEvent& e);
    private:
        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        LayerStack m_LayerStack;
        Timestep m_Timestep;
        float m_LastFrameTime = 0.0f;
    private:
        static Application* s_Instance;
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}